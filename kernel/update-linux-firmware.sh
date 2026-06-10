#!/bin/sh

cd "$(dirname "$(realpath "$0")")"

rm -rf ./linux-firmware-src
rm -rf ./linux-firmware
rm -f ./linux-firmware.tar.zst

echo ""
git clone https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git ./linux-firmware-src
rm -rf ./linux-firmware-src/.git/

echo ""
echo "make install-zst"
sh -c "cd ./linux-firmware-src && make DESTDIR=\"$(realpath ./linux-firmware)\" install-zst" 2>&1 1>/dev/null

echo ""
echo "make dedup"
sh -c "cd ./linux-firmware-src && make DESTDIR=\"$(realpath ./linux-firmware)\" dedup" 2>&1 1>/dev/null

mv ./linux-firmware/lib/firmware/* ./linux-firmware
rm -r ./linux-firmware/lib

echo ""
echo "tar --zstd -cf ./linux-firmware.tar.zst ./linux-firmware"
tar --zstd -cf ./linux-firmware.tar.zst ./linux-firmware

rm -r ./linux-firmware-src
rm -r ./linux-firmware

echo ""
echo "Done!"
echo ""
