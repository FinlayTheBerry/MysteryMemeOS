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

def GetKernelDir():
    with open("/proc/sys/kernel/osrelease") as f:
        unamer = f.read().strip()
        return f"/usr/lib/modules/{unamer}"

def ParseProcModules(modulesFilePath="/proc/modules"):
    output = []
    with open(modulesFilePath) as f:
        for line in f.read().strip().splitlines():
            mod_name, mem_size, ref_count, dependencies, load_state, mem_offset = line.split(" ")[:6]
            dependencies = [] if dependencies == "-" else dependencies.removesuffix(",").split(",")
            output.append({ "mod_name": mod_name, "mem_size": mem_size, "ref_count": ref_count, "dependencies": dependencies, "load_state": load_state, "mem_offset": mem_offset })
    return output
def SortModules(modules):
    sortedModules = []
    for module in modules:
        inserted = False
        dependencies = module['dependencies'][:]
        for i in range(len(sortedModules)):
            if sortedModules[i]['mod_name'] in dependencies:
                dependencies.remove(sortedModules[i]['mod_name'])
            if len(dependencies) == 0:
                inserted = True
                sortedModules.insert(i, module)
                break
        if not inserted:
            sortedModules.append(module)
    return sortedModules
def AddModuleFilePaths(modules, modulesDepFilePath=None):
    kernel_dir = GetKernelDir()
    modules = copy.deepcopy(modules)
    if modulesDepFilePath == None:
        modulesDepFilePath = os.path.join(kernel_dir, "modules.dep")
    mod_paths = {}
    with open(modulesDepFilePath) as f:
        modulesDep = f.read().strip()
        for file_path in modulesDep.splitlines():
            file_path = os.path.join(kernel_dir, file_path.partition(":")[0])
            file_mod_name = os.path.basename(file_path).partition(".")[0].replace("-", "_")
            mod_paths[file_mod_name] = file_path
    for module in modules:
        if not module['mod_name'] in mod_paths:
            raise Exception(f"Matching file path could not be found for module {module['mod_name']}.")
        else:
            module['file_path'] = mod_paths[module['mod_name']]
    return modules

def ParseLoadedFirmware(firmwareBaseDir="/usr/lib/firmware"):
    firmwares = []
    dmesg = RunCommand("dmesg", capture=True)
    for line in dmesg.splitlines():
        if " Loaded FW: " in line:
            file_path = glob.glob(os.path.join(firmwareBaseDir, f"{line.split(' ')[8].removesuffix(',')}*"))[0]
            in_mem_hash = line.split(" ")[10].removesuffix(",")
            firmwares.append({ "file_path": file_path, "in_mem_hash": in_mem_hash })
    return firmwares

modules = ParseProcModules()
modules = SortModules(modules)
modules = AddModuleFilePaths(modules)

firmwares = ParseLoadedFirmware()

modules_size = 0
for module in modules:
    modules_size += os.stat(module['file_path']).st_size

firmware_size = 0
for firmware in firmwares:
    firmware_size += os.stat(firmware['file_path']).st_size

kernel_size = os.stat(os.path.join(GetKernelDir(), "vmlinuz")).st_size

total = modules_size + firmware_size + kernel_size
print(f"modules: {modules_size / (1024 * 1024):.1f}MB, firmware: {firmware_size / (1024 * 1024):.1f}MB, kernel: {kernel_size / (1024 * 1024):.1f}MB, total: {total / (1024 * 1024):.1f}MB")
