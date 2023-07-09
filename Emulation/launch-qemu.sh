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
    exit 1
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
SMP="-smp 2"
ACCELERATOR=""

# 3. Memory override
MEMORY="-m 2G"

# 4. BIOS/UEFI settings
EFI_FLASH_PATH=""
EFI_VARS_PATH=""

EFI_FLASH_DRV=""
EFI_VARS_DRV=""

# 5. Device selection
# 5.1. Input
INPUT_DEV=""

# 5.2. Network
NETWORK_DEV=""

# 5.3 Storage
STORAGE_DEV=""

# 5.4. Display
DISPLAY_DEV="-device ramfb"

# 5.5. Sound
SOUND_DEV=""

# 5.6. Misc
MISC_DEV=""

# 6. Drive settings (a.k.a disk images)
DISK_PATH=""

MAIN_DRV=""

# END Guest

# Launch the machine
set -x

${QEMU_BIN} ${NAME} ${MACHINE} ${CPU} ${SMP} ${ACCELERATOR} ${MEMORY} \
    ${EFI_FLASH_DRV} ${EFI_VARS_DRV} ${INPUT_DEV} ${NETWORK_DEV} ${STORAGE_DEV} ${DISPLAY_DEV} ${SOUND_DEV} ${MISC_DEV} \
    ${CD_DRV} ${MAIN_DRV} $*
