#!/bin/env python3

import copy
import datetime
import grp
import os
import pwd
import time
import urllib.request
import json
import subprocess
import glob

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
        return "NaN"
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

def GetKernelDir():
    with open("/proc/sys/kernel/osrelease") as f:
        unamer = f.read().strip()
        return f"/usr/lib/modules/{unamer}"
def FormatBytes(count):
    units = [ "TB", "GB", "MB", "KB" ]
    for i in range(len(units)):
        threshold = 1024 ** (len(units) - i)
        if count >= threshold:
            return f"{count / threshold:.01f}{units[i]}"
    return str(count)

def ParseProcModules(modulesFilePath="/proc/modules"):
    output = []
    with open(modulesFilePath) as f:
        for line in f.read().strip().splitlines():
            modName, memSize, refCount, dependencies, loadState, memOffset = line.split(" ")[:6]
            dependencies = [] if dependencies == "-" else dependencies.removesuffix(",").split(",")
            output.append({ "modName": modName, "memSize": memSize, "refCount": refCount, "dependencies": dependencies, "loadState": loadState, "memOffset": memOffset })
    return output
def SortModules(modules):
    modulesByName = { module['modName']: module for module in modules }
    sortedModules = []
    def insertMod(module):
        for dependencyName in module['dependencies']:
            if dependencyName in modulesByName:
                insertMod(modulesByName[dependencyName])
        if module['modName'] in modulesByName:
            del modulesByName[module['modName']]
            sortedModules.append(module)
    while(len(modulesByName) > 0):
        insertMod(list(modulesByName.values())[0])
    return sortedModules
def AddModuleFilePaths(modules, kernelDir=None):
    modules = copy.deepcopy(modules)
    if kernelDir == None:
        kernelDir = GetKernelDir()
    modulesDepFilePath = os.path.join(kernelDir, "modules.dep")
    modPaths = {}
    with open(modulesDepFilePath) as f:
        modulesDep = f.read().strip()
        for filePath in modulesDep.splitlines():
            filePath = os.path.join(kernelDir, filePath.partition(":")[0])
            fileModName = os.path.basename(filePath).partition(".")[0].replace("-", "_")
            modPaths[fileModName] = filePath
    for module in modules:
        if not module['modName'] in modPaths:
            raise Exception(f"Matching file path could not be found for module {module['modName']}.")
        else:
            module['filePath'] = modPaths[module['modName']]
    return modules

def ParseLoadedFirmware(firmwareDir="/usr/lib/firmware"):
    firmwares = []
    dmesg = RunCommand("dmesg", capture=True)
    for line in dmesg.splitlines():
        if " Loaded FW: " in line:
            line = line.partition(" Loaded FW: ")[2]
            firmwareName = line.split(" ")[0].removesuffix(',')
            filePath = glob.glob(os.path.join(firmwareDir, f"{firmwareName}*"))[0]
            inMemHash = line.split(" ")[2].removesuffix(",")
            firmwares.append({ "filePath": filePath, "inMemHash": inMemHash })
    return firmwares

def InstallBase(baseDirPath):
    InstallDirectory(baseDirPath)
    
    InstallDirectory(os.path.join(baseDirPath, "proc"))
    InstallDirectory(os.path.join(baseDirPath, "sys"))
    InstallDirectory(os.path.join(baseDirPath, "dev"))

    InstallDirectory(os.path.join(baseDirPath, "usr"))
    InstallDirectory(os.path.join(baseDirPath, "usr/bin"))
    InstallDirectory(os.path.join(baseDirPath, "usr/lib"))
    
    RunCommand(f"ln -s usr/bin \'{os.path.join(baseDirPath, 'bin')}\'")
    RunCommand(f"ln -s usr/bin \'{os.path.join(baseDirPath, 'sbin')}\'")
    RunCommand(f"ln -s bin \'{os.path.join(baseDirPath, 'usr/sbin')}\'")
    RunCommand(f"ln -s usr/lib \'{os.path.join(baseDirPath, 'lib')}\'")
    RunCommand(f"ln -s usr/lib \'{os.path.join(baseDirPath, 'lib64')}\'")
    RunCommand(f"ln -s lib \'{os.path.join(baseDirPath, 'usr/lib64')}\'")

kernelDir = GetKernelDir()
firmwareDir="/usr/lib/firmware"

modules = ParseProcModules()
modules = SortModules(modules)
modules = AddModuleFilePaths(modules, kernelDir=kernelDir)

firmwares = ParseLoadedFirmware(firmwareDir=firmwareDir)

InstallBase("./cpio")

InstallDirectory("./cpio/usr/lib/modules")
modList = ""
modulesSize = 0
for module in modules:
    modulesSize += os.stat(module['filePath']).st_size

    relPath = os.path.relpath(module['filePath'], kernelDir)
    
    dirPath = os.path.dirname(relPath)
    InstallDirectory(os.path.join("./cpio/usr/lib/modules", dirPath), andParents=True)
    
    targetPath = os.path.join('./cpio/usr/lib/modules', relPath)
    if targetPath.endswith(".zst"):
        targetPath = targetPath.removesuffix(".zst")
        RunCommand(f"zstd -d \'{module['filePath']}\' -o \'{targetPath}\'")
    else:
        RunCommand(f"cp \'{module['filePath']}\' \'{targetPath}\'")
    modList += targetPath.removeprefix("./cpio") + "\n"
InstallFile("./cpio/modlist", modList)

InstallDirectory("./cpio/usr/lib/firmware")
firmwaresSize = 0
for firmware in firmwares:
    firmwaresSize += os.stat(firmware['filePath']).st_size

    relPath = os.path.relpath(firmware['filePath'], firmwareDir)
    
    dirPath = os.path.dirname(relPath)
    InstallDirectory(os.path.join("./cpio/usr/lib/firmware", dirPath), andParents=True)

    targetPath = os.path.join('./cpio/usr/lib/firmware', relPath)
    if targetPath.endswith(".zst"):
        targetPath = targetPath.removesuffix(".zst")
        RunCommand(f"zstd -d \'{firmware['filePath']}\' -o \'{targetPath}\'")
    else:
        RunCommand(f"cp \'{firmware['filePath']}\' \'{targetPath}\'")

kernelPath = os.path.join(GetKernelDir(), "vmlinuz")
kernelSize = os.stat(kernelPath).st_size

busyboxSize = os.stat("/usr/bin/busybox").st_size
RunCommand(f"cp \'/usr/bin/busybox\' \'./cpio/usr/bin/busybox\'")
for appletName in RunCommand("busybox --list", capture=True).splitlines():
    targetPath = os.path.join('./cpio/usr/bin', appletName)
    if not os.path.exists(targetPath):
        RunCommand(f"ln -s busybox \'{os.path.join('./cpio/usr/bin', appletName)}\'")

mysterySize = os.stat("./mystery").st_size
RunCommand(f"cp \'./mystery\' \'./cpio/mystery\'")

initSize = os.stat("./init").st_size
RunCommand(f"cp \'./init\' \'./cpio/init\'")

totalSize = modulesSize + firmwaresSize + kernelSize + busyboxSize + mysterySize + initSize

print(f"modules: {FormatBytes(modulesSize)}, firmware: {FormatBytes(firmwaresSize)}, kernel: {FormatBytes(kernelSize)}, busybox: {FormatBytes(busyboxSize)}, mystery: {FormatBytes(mysterySize)}, init: {FormatBytes(initSize)}, total: {FormatBytes(totalSize)}")

RunCommand(f"cd cpio && find . -print | cpio -ov --format=newc --owner=+0:+0 --file='../mystery.img'", echo=True)

RunCommand(f"zstd mystery.img", echo=True)

ukifyConf = f"""[UKI]
Cmdline=quite
Linux={kernelPath}
Initrd=mystery.img.zst
"""
InstallFile("ukify.conf", ukifyConf)
RunCommand(f"ukify -c ukify.conf build -o mystery.efi", echo=True)

print("Done!")
