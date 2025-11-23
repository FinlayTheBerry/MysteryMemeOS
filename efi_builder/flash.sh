#!/bin/bash
set -euox pipefail
cd "$(dirname "$(realpath "$0")")"

if [[ ! -f "./obj/mystery.efi" ]]; then
    echo "Mystery.efi could not be found."
    exit 1
fi

if [[ "$(blkid --uuid 44C2-8EC9)" == "" ]]; then
    echo "Error flash drive could not be found."
    exit 1
fi

if [[ "$(id -u)" != "0" ]]; then
    echo "Error flashing requires root."
    exit 1
fi

rm -rf ./obj/flash_drive
mkdir -p ./obj/flash_drive
mount "$(blkid --uuid 44C2-8EC9)" "$(realpath ./obj/flash_drive)"
rm -rf ./obj/flash_drive/{*,.[!.]*,..?*}
mkdir -p ./obj/flash_drive/EFI/BOOT/
cp ./obj/mystery.efi ./obj/flash_drive/EFI/BOOT/BOOTX64.EFI
chown -R +0:+0 ./obj/flash_drive/
umount "$(realpath ./obj/flash_drive)"