#!/bin/env sh

sudo rm -rf ./firmware
sudo rm -rf ./pacstrap_root
sudo mkdir ./pacstrap_root
sudo pacstrap ./pacstrap_root linux-firmware-amdgpu linux-firmware-nvidia linux-firmware-intel linux-firmware-radeon linux-firmware-cirrus linux-firmware-other
sudo mv ./pacstrap_root/usr/lib/firmware ./
sudo chown finlaytheberry:finlaytheberry -hR ./firmware
sudo rm -rf ./pacstrap_root