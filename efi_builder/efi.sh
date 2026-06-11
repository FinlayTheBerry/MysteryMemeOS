#!/bin/bash
set -euo pipefail
cd "$(dirname "$(realpath "$0")")"
mkdir -p ./obj

compress="true"

# copy mystery and init script
cp ../payload/bin/mystery ./obj/cpio/usr/bin/mystery
chmod 700 ./obj/cpio/usr/bin/mystery
cp ./resources/init.sh ./obj/cpio/init
chmod 700 ./obj/cpio/init
cp ./resources/rickrollmini.mkv ./obj/cpio/rickroll.mkv
chmod 700 ./obj/cpio/rickroll.mkv

# Generate cpio image
cd ./obj/cpio
find . -print | cpio -ov --format=newc --owner=+0:+0 --file="../mystery.img"
cd ../../

# Compress image
if [[ "$compress" == "true" ]]; then
    zstd ./obj/mystery.img # -22 --ultra -T0
fi

# Build uki efi file
echo "[UKI]" > ./obj/ukify.conf
echo "Cmdline=splash quite" >> ./obj/ukify.conf
echo "splash=./resources/splash.bmp" >> ./obj/ukify.conf
echo "Linux=$(realpath ./obj/vmlinuz)" >> ./obj/ukify.conf
if [[ "$compress" == "true" ]]; then
    echo "Initrd=$(realpath ./obj/mystery.img.zst)" >> ./obj/ukify.conf
else
    echo "Initrd=$(realpath ./obj/mystery.img)" >> ./obj/ukify.conf
fi
ukify -c ./obj/ukify.conf build -o ./obj/mystery.efi

meson setup .. \
  --buildtype=minsize \
  -Ddefault_library=static \