#!/bin/sh

cd "$(dirname "$(realpath "$0")")"

rm -f ./linux-kernel-src.tar.xz
rm -rf ./linux-kernel-src
rm -rf ./linux-kernel
rm -f ./linux-kernel.tar.zst

echo ""
linuxKernelSrcUrl=$(curl -s https://kernel.org/releases.json | jq -r '.releases[] | select(.moniker == "stable") | .source')
echo "curl -o ./linux-kernel-src.tar.xz ./$linuxKernelSrcUrl"
curl --progress-bar -o ./linux-kernel-src.tar.xz "$linuxKernelSrcUrl"

echo ""
mkdir ./linux-kernel-src
echo "tar -xf ./linux-kernel-src.tar.xz -C ./linux-kernel-src --strip-components=1"
tar -xf ./linux-kernel-src.tar.xz -C ./linux-kernel-src --strip-components=1

echo ""
echo "cp ./config ./linux-kernel-src/.config && make oldconfig"
cp ./config ./linux-kernel-src/.config
sh -c "cd ./linux-kernel-src && make oldconfig"

echo ""
echo "make -j$(nproc) KBUILD_BUILD_USER=linux KBUILD_BUILD_HOST=mystery EXTRAVERSION=-mystery"
sh -c "cd ./linux-kernel-src && make -j$(nproc) KBUILD_BUILD_USER=linux KBUILD_BUILD_HOST=mystery EXTRAVERSION=-mystery"

echo ""
echo "make INSTALL_MOD_PATH=../linux-kernel INSTALL_MOD_STRIP=1 modules_install"
sh -c "cd ./linux-kernel-src && make INSTALL_MOD_PATH=../linux-kernel INSTALL_MOD_STRIP=1 modules_install"

mv ./linux-kernel/lib/modules/*/* ./linux-kernel/
rm ./linux-kernel/build
rm -r ./linux-kernel/lib
cp ./linux-kernel-src/arch/x86/boot/bzImage ./linux-kernel/vmlinuz

echo ""
echo "tar --zstd -cf ./linux-kernel.tar.zst ./linux-kernel"
tar --zstd -cf ./linux-kernel.tar.zst ./linux-kernel

#rm ./linux-kernel-src.tar.xz
#rm -r ./linux-kernel-src
#rm -r ./linux-kernel

echo "
echo "Done!"
echo ""
