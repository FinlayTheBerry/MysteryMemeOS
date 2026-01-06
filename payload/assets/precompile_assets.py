#!/usr/bin/env python3
import os
import subprocess
import sys
import struct
from PIL import Image
import wave

def WriteFile(filePath, contents, binary=False):
    filePath = os.path.realpath(os.path.expanduser(filePath))
    os.makedirs(os.path.dirname(filePath), exist_ok=True)
    with open(filePath, "wb" if binary else "w", encoding=(None if binary else "UTF-8")) as file:
        file.write(contents)
def ReadFile(filePath, defaultContents=None, binary=False):
    filePath = os.path.realpath(os.path.expanduser(filePath))
    if not os.path.exists(filePath):
        if defaultContents != None:
            return defaultContents
    with open(filePath, "rb" if binary else "r", encoding=(None if binary else "UTF-8")) as file:
        return file.read()
def RunCommand(command, echo=False, capture=False, input=None, check=True, env=None):
    if echo and capture:
        raise Exception("Command cannot be run with both echo and capture.")
    result = subprocess.run(command, stdout=(None if echo else subprocess.PIPE), stderr=(None if echo else subprocess.STDOUT), input=input, env=env, check=False, shell=True, text=True)
    if check and result.returncode != 0:
        print(result.stdout)
        raise Exception(f"Sub-process returned non-zero exit code.\nExitCode: {result.returncode}\nCmdLine: {command}")
    if capture and not check:
        return result.stdout.strip(), result.returncode
    elif capture:
        return result.stdout.strip()
    elif not check:
        return result.returncode
    else:
        return

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
        mysterysong_sample_count = mysterysong.getnframes()
        mysterysong_sample_rate = mysterysong.getframerate()
        mysterysong_channel_count = mysterysong.getnchannels()
        mysterysong_bytes_per_sample = mysterysong.getsampwidth()
        mysterysong_payload = mysterysong.readframes(mysterysong_sample_count)
        assets_h += "extern const uint32_t mysterysong_sample_count;\n"
        assets_h += "extern const uint32_t mysterysong_sample_rate;\n"
        assets_h += "extern const uint32_t mysterysong_channel_count;\n"
        assets_h += "extern const uint32_t mysterysong_bytes_per_sample;\n"
        assets_h += "extern const size_t mysterysong_length;\n"
        assets_h += "extern const uint8_t mysterysong_buffer[];\n"
        assets_h += "\n"
        assets_c += f"const uint32_t mysterysong_sample_count = {mysterysong_sample_count};\n"
        assets_c += f"const uint32_t mysterysong_sample_rate = {mysterysong_sample_rate};\n"
        assets_c += f"const uint32_t mysterysong_channel_count = {mysterysong_channel_count};\n"
        assets_c += f"const uint32_t mysterysong_bytes_per_sample = {mysterysong_bytes_per_sample};\n"
        assets_c += f"const size_t mysterysong_length = {len(mysterysong_payload)};\n"
        assets_c += f"const uint8_t mysterysong_buffer[] = {{ {", ".join([f"0x{b:02x}" for b in mysterysong_payload])} }};"
    os.remove("mysterysong.wav")

    assets_h += "#endif"
    WriteFile("./assets.h", assets_h)
    WriteFile("./assets.c", assets_c)
    return 0
sys.exit(Main())