#!/usr/bin/env python3
import datetime
import json
import os
import subprocess
import sys
import time
import urllib
import wave

# region Epsilon PYSTDLIB
def RealPath(filePath):
    return os.path.realpath(os.path.expanduser(filePath))
def GetScriptDir():
    return os.path.dirname(RealPath(__file__))
def InstallFile(filePath, contents, mode=0o644, user="~", group="~", binary=False):
    filePath = RealPath(filePath)
    uid = os.getuid() if user == "~" else (int(user[1:]) if user.startswith("+") else pwd.getpwnam(user).pw_uid)
    gid = os.getgid() if group == "~" else (int(group[1:]) if group.startswith("+") else grp.getgrnam(group).gr_gid)
    # open or create file with set mode
    fd = os.open(filePath, os.O_CREAT | os.O_WRONLY | os.O_TRUNC, mode)
    # chown to ensure other users can't set perms
    os.fchown(fd, uid, gid)
    # chmod to ensure correct mode despite umask
    os.fchmod(fd, mode)
    with open(fd, "wb" if binary else "w", encoding=None if binary else "utf-8") as f:
        f.write(contents)
def InstallDirectory(dirPath, mode=0o755, user="~", group="~", andParents=False):
    dirPath = RealPath(dirPath)
    # Parent mode is only set if not exists because otherwise mode on / would be changed
    if andParents and not os.path.exists(os.path.dirname(dirPath)):
        InstallDirectory(os.path.dirname(dirPath), mode=mode, user=user, group=group, andParents=andParents)
    uid = os.getuid() if user == "~" else (int(user[1:]) if user.startswith("+") else pwd.getpwnam(user).pw_uid)
    gid = os.getgid() if group == "~" else (int(group[1:]) if group.startswith("+") else grp.getgrnam(group).gr_gid)
    # create dir if not exists
    if not os.path.isdir(dirPath):
        os.mkdir(dirPath, mode)
    # chown to ensure other users can't set perms
    os.chown(dirPath, uid, gid)
    # chmod to ensure correct mode despite umask
    os.chmod(dirPath, mode)
def ReadFile(filePath, defaultContents=None, binary=False):
    filePath = RealPath(filePath)
    try:
        with open(filePath, "rb" if binary else "r", encoding=None if binary else "utf-8") as f:
            return f.read()
    except FileNotFoundError:
        if defaultContents != None:
            return defaultContents
        else:
            raise
    if defaultContents != None and not os.path.exists(filePath):
        return defaultContents
def SerializeJson(obj, compact=False):
    return json.dumps(obj, separators=(',', ':') if compact else None, indent=None if compact else 4)
def DeserializeJson(jsonString):
    return json.loads(jsonString)
def GetEpoch():
    return time.time()
def FormatEpoch(epoch):
    if epoch == float("inf") or epoch == float("-inf"):
        return "NONE_TIME"
    timestamp = datetime.datetime.fromtimestamp(epoch)
    return timestamp.strftime("%I:%M%p %m/%d").lower()
def RunCommand(command, echo=False, capture=False, noerr=False, input=None, check=True, env=None):
    if echo and capture:
        raise Exception("Command cannot be run with both echo and capture.")
    result = subprocess.run(command, stdout=(None if echo else subprocess.PIPE), stderr=(subprocess.DEVNULL if noerr else (None if echo else subprocess.STDOUT)), input=input, env=env, check=False, shell=True, text=True)
    if check and result.returncode != 0:
        raise Exception(f"Sub-process returned non-zero exit code.\nExitCode: {result.returncode}\nCmdLine: {command}{'\n\n' + result.stdout if not echo else ''}")
    if capture and not check:
        return result.stdout.strip(), result.returncode
    elif capture:
        return result.stdout.strip()
    elif not check:
        return result.returncode
    else:
        return
def Fetch(url, headers=None, binary=False):
    if headers == None:
        with urllib.request.urlopen(url) as response:
            payload = response.read()
            if not binary:
                payload = payload.decode(response.headers.get_content_charset() or "utf-8")
            return payload
    else:
        with urllib.request.urlopen(url, headers=headers) as response:
            payload = response.read()
            if not binary:
                payload = payload.decode(response.headers.get_content_charset() or "utf-8")
            return payload
os.chdir(GetScriptDir())
# endregion

try:
    from PIL import Image
except:
    print(f"python3 -m venv venv")
    RunCommand(f"python3 -m venv venv", echo=True)
    print(f"source venv/bin/activate && python3 -m pip install --upgrade pip")
    RunCommand(f"source venv/bin/activate && python3 -m pip install --upgrade pip", echo=True)
    print(f"source venv/bin/activate && python3 -m pip install -r requirements.txt")
    RunCommand(f"source venv/bin/activate && python3 -m pip install -r requirements.txt", echo=True)
    print(f"source venv/bin/activate && python3 '{RealPath(__file__)}'")
    sys.exit(RunCommand(f"source venv/bin/activate && python3 '{RealPath(__file__)}'", check=False, echo=True))

def Main():
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    
    assets_h = ""
    assets_h += "#ifndef assets_h\n"
    assets_h += "#define assets_h\n"
    assets_h += "#include <stdint.h>\n"
    assets_h += "#include <stddef.h>\n"
    assets_h += "\n"
    assets_c = ""
    assets_c += "#include <stdint.h>\n"
    assets_c += "#include <stddef.h>\n"
    assets_c += "\n"

    with Image.open("./mysteryimage.png").convert("RGBA") as mysteryimage:
        mysteryimage_width, mysteryimage_height = mysteryimage.size
        mysteryimage_payload = mysteryimage.tobytes("raw", "RGBA")
        assets_h += "extern const uint32_t mysteryimage_width;\n"
        assets_h += "extern const uint32_t mysteryimage_height;\n"
        assets_h += "extern const uint32_t mysteryimage_stride;\n"
        assets_h += "extern const size_t mysteryimage_length;\n"
        assets_h += "extern const uint8_t mysteryimage_buffer[];\n"
        assets_h += "\n"
        assets_c += f"const uint32_t mysteryimage_width = {mysteryimage_width};\n"
        assets_c += f"const uint32_t mysteryimage_height = {mysteryimage_height};\n"
        assets_c += f"const uint32_t mysteryimage_stride = {mysteryimage_width * 4};\n"
        assets_c += f"const size_t mysteryimage_length = {len(mysteryimage_payload)};\n"
        assets_c += f"const uint8_t mysteryimage_buffer[] = {{ {", ".join([f"0x{b:02x}" for b in mysteryimage_payload])} }};\n"
        assets_c += "\n"

    RunCommand("ffmpeg -i mysterysong.xm -ar 44100 -ac 2 -c:a pcm_s16le -map_metadata -1 -y mysterysong.wav")
    with wave.open("mysterysong.wav", "r") as mysterysong:
        mysterysong_frame_count = mysterysong.getnframes()
        mysterysong_frame_rate = mysterysong.getframerate()
        mysterysong_channel_count = mysterysong.getnchannels()
        mysterysong_bytes_per_sample = mysterysong.getsampwidth()
        mysterysong_payload = mysterysong.readframes(mysterysong_frame_count)
        assets_h += "extern const uint32_t mysterysong_frame_count;\n"
        assets_h += "extern const uint32_t mysterysong_frame_rate;\n"
        assets_h += "extern const uint32_t mysterysong_channel_count;\n"
        assets_h += "extern const uint32_t mysterysong_bytes_per_sample;\n"
        assets_h += "extern const size_t mysterysong_length;\n"
        assets_h += "extern const uint8_t mysterysong_buffer[];\n"
        assets_h += "\n"
        assets_c += f"const uint32_t mysterysong_frame_count = {mysterysong_frame_count};\n"
        assets_c += f"const uint32_t mysterysong_frame_rate = {mysterysong_frame_rate};\n"
        assets_c += f"const uint32_t mysterysong_channel_count = {mysterysong_channel_count};\n"
        assets_c += f"const uint32_t mysterysong_bytes_per_sample = {mysterysong_bytes_per_sample};\n"
        assets_c += f"const size_t mysterysong_length = {len(mysterysong_payload)};\n"
        assets_c += f"const uint8_t mysterysong_buffer[] = {{ {", ".join([f"0x{b:02x}" for b in mysterysong_payload])} }};"
    os.remove("mysterysong.wav")

    assets_h += "#endif"
    InstallFile("./assets.h", assets_h)
    InstallFile("./assets.c", assets_c)
    return 0
sys.exit(Main())