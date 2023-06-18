#!/usr/bin/env bash

#
# Host
#

QEMU_BIN=qemu-system-aarch64

if [[ -x "$(command -v ${QEMU_BIN})" ]]; then
    echo "[x] Found ${QEMU_BIN}"
    ${QEMU_BIN} --version
else
    echo "[!] ${QEMU_BIN} not found on the system! Make sure it is installed"
    exit -1
fi

# END Host

#
# Guest
#

# 0. Meta
NAME="-name My-Hypothetical-Machine"

# 1. Machine selection
MACHINE="-machine virt"

# 2. CPU override
CPU="-cpu cortex-a72"
SMP=""
ACCELERATOR=""

# 3. Memory override
MEMORY="-m 8G"

# 4. BIOS/UEFI settings

# 5. Device selection
# 5.1. Input
INPUT_DEV="-device usb-ehci -device usb-kbd"

# 5.2. Network
NETWORK_DEV=""

# 5.3 Storage
STORAGE_DEV="-device sdhci-pci -device sd-card,drive=sdcard0"

# 5.4. Display
DISPLAY_DEV="-device ramfb"

# 5.5. Sound
SOUND_DEV=""

# 5.6. Misc
MISC_DEV=""

# 6. Drive settings (a.k.a disk images)
DISK_PATH="./disk0.qcow2"

MAIN_DRV="-drive id=sdcard0,if=none,format=qcow2,file=${DISK_PATH}"

# END Guest

# Launch the machine
set -x

${QEMU_BIN} ${NAME} ${MACHINE} ${CPU} ${SMP} ${ACCELERATOR} ${MEMORY} \
    ${EFI_FLASH_DRV} ${EFI_VARS_DRV} ${INPUT_DEV} ${NETWORK_DEV} ${STORAGE_DEV} ${DISPLAY_DEV} ${SOUND_DEV} ${MISC_DEV} \
    ${CD_DRV} ${MAIN_DRV} $*
