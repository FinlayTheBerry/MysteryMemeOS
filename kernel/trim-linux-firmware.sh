#!/bin/sh

cd "$(dirname "$(realpath "$0")")"

rm -f ./linux-firmware-trimmed.tar.zst
rm -rf ./linux-firmware

echo ""
echo "tar -xf linux-firmware.tar.zst"
tar -xf linux-firmware.tar.zst

echo ""
echo "Trimming Modules..."
rm ./linux-firmware/iwlwifi-*
rm -r ./linux-firmware/intel/iwlwifi
rm -r ./linux-firmware/mellanox
rm -r ./linux-firmware/qcom
rm -r ./linux-firmware/mrvl
rm -r ./linux-firmware/mediatek
rm -r ./linux-firmware/ath10k
rm -r ./linux-firmware/ath11k
rm -r ./linux-firmware/ath12k

echo ""
echo "tar --zstd -cf ./linux-firmware-trimmed.tar.zst ./linux-firmware"
tar --zstd -cf ./linux-firmware-trimmed.tar.zst ./linux-firmware

rm -r ./linux-firmware

echo ""
echo "Done!"
echo ""
