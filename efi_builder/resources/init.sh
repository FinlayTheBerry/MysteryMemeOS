#!/usr/bin/busybox ash
reboot_sysrq() {
    echo "1" >/proc/sys/kernel/sysrq 
    echo "e" >/proc/sysrq-trigger
    echo "s" >/proc/sysrq-trigger
    echo "u" >/proc/sysrq-trigger
    sleep 1
    echo "o" >/proc/sysrq-trigger
    echo "Target Reached - Reboot Requested"
    while true; do
        :
    done
}

devmod() {
    devpaths=$(ls /sys/bus/*/devices/*/modalias 2>/dev/null)
    for devpath in $devpaths; do
        alias=$(cat "$devpath")
        if [ -n "$alias" ]; then
            modules=$(/usr/bin/modprobe --quiet --resolve-alias "$alias" 2>/dev/null)
            if [ -n "$modules" ]; then
                for module in $modules; do
                    /usr/bin/modprobe "$module" 1>/dev/null 2>&1
                done
            fi
        fi
    done
}

export PATH="/usr/bin"
mount -t proc proc /proc -o noatime,nodiratime,norelatime,nodev,noexec,nosuid
mount -t sysfs sys /sys -o noatime,nodiratime,norelatime,nodev,noexec,nosuid
mount -t devtmpfs dev /dev -o noatime,nodiratime,norelatime,noexec,nosuid
if [ -e /sys/firmware/efi ]; then
    mount -t efivarfs efivarfs /sys/firmware/efi/efivars -o noatime,nodiratime,norelatime,nodev,noexec,nosuid
fi
echo "0" >/proc/sys/kernel/ctrl-alt-del
echo "0" >/proc/sys/kernel/sysrq
/usr/bin/depmod --all
devmod
mystery --novideo
reboot_sysrq