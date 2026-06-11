#!/bin/sh

cd "$(dirname "$(realpath "$0")")"

rm -f ./linux-kernel-trimmed.tar.zst
rm -rf ./linux-kernel

echo ""
echo "tar -xf linux-kernel.tar.zst"
tar -xf linux-kernel.tar.zst

echo ""
echo "Trimming Modules..."
rm -r ./linux-kernel/kernel/net
rm -r ./linux-kernel/kernel/fs
rm -r ./linux-kernel/kernel/drivers/net
rm -r ./linux-kernel/kernel/drivers/usb
rm -r ./linux-kernel/kernel/drivers/input
rm -r ./linux-kernel/kernel/drivers/hid
rm -r ./linux-kernel/kernel/drivers/bluetooth
rm -r ./linux-kernel/kernel/drivers/nfc
rm -r ./linux-kernel/kernel/drivers/thunderbolt
rm -r ./linux-kernel/kernel/drivers/firewire
rm -r ./linux-kernel/kernel/drivers/cdrom
rm -r ./linux-kernel/kernel/arch/x86/kvm
rm -r ./linux-kernel/kernel/kernel
rm -r ./linux-kernel/kernel/drivers/media/usb

echo ""
echo "tar --zstd -cf ./linux-kernel-trimmed.tar.zst ./linux-kernel"
tar --zstd -cf ./linux-kernel-trimmed.tar.zst ./linux-kernel

rm -r ./linux-kernel

echo ""
echo "Done!"
echo ""
