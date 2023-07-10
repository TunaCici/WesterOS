# COLORS FOR DAYZZZ
RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[0;33m
BLUE=\033[0;34m
MAGENTA=\033[0;35m
CYAN=\033[0;36m
NC=\033[0m

# Project
PROJECT_NAME=WesterOS
PROJECT_DIR=${shell pwd}
BUILD_DIR=Build

# Target architecture & Toolchain
TARGET_ARCH=aarch64
TARGET_TRIPLE=aarch64-none-elf
TOOLCHAIN_PATH=Toolchain/arm-gnu-toolchain-12.2.rel1-darwin-arm64-aarch64-none-elf

# Binaries
CC=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc
CXX=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++
AS=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-as
LD=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld
AR=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ar
OBJCOPY=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-objcopy
OBJDUMP=${TOOLCHAIN_PATH}/bin/aarch64-none-elf-objdump

# Flags
INCLUDES=-I Kernel/Include
CFLAGS=${INCLUDES} -Wall -Wextra -ffreestanding -nostdlib -std=gnu99 -O2
LDFLAGS=

# QEMU
QEMU_SCRIPT=Emulation/launch-qemu.sh

# Object files
OBJS = \
	Kernel/Start.o \
	Kernel/Main.o \
	Kernel/Library/LibKern/Console.o \

KERN_OBJS = Entry.o ${OBJS}
LDSCRIPT = Kernel/kernel.ld

# Architectur dependant files
ifeq (${TARGET_ARCH},aarch64)
	ENTRY = Kernel/Arch/ARM64/Entry.S
else
	${error Unsupported target architecture ${TARGET_ARCH}}
endif

# Targets
default: all

Entry.o: ${ENTRY}
	@echo "AS ${ENTRY}"
	@${AS} ${ENTRY} -o ${BUILD_DIR}/Entry.o
	@echo "AS ${ENTRY} ${GREEN}ok${NC}"

${OBJS}: %.o: %.c
	@echo "CC $<"
	@${CC} ${CFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"

kernel: ${KERN_OBJS}
	@echo "LINK ${KERN_OBJS} -T ${LDSCRIPT}"
	@${LD} ${LDFLAGS} -T ${LDSCRIPT} -o ${BUILD_DIR}/kernel.elf ${addprefix ${BUILD_DIR}/, $(notdir ${KERN_OBJS})}
	@echo "LINK ${KERN_OBJS} ${GREEN}ok${NC}"

debug:
	@echo "OBJDUMP ${BUILD_DIR}/kernel.elf"
	@${OBJDUMP} -S ${BUILD_DIR}/kernel.elf > ${BUILD_DIR}/kernel.asm
	@${OBJDUMP} -t ${BUILD_DIR}/kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > ${BUILD_DIR}/kernel.sym
	@echo "OBJDUMP ${BUILD_DIR}/kernel.elf ${GREEN}ok${NC}"

	@echo "OBJDUMP ${BUILD_DIR}/Entry.o"
	@${OBJDUMP} -S ${BUILD_DIR}/Entry.o > ${BUILD_DIR}/entry.asm
	@echo "OBJDUMP ${BUILD_DIR}/Entry.o ${GREEN}ok${NC}"

all:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "${shell ${CC} --version | head -n 1}"
	@echo "${shell ${CXX} --version | head -n 1}"
	@echo "${shell ${LD} --version | head -n 1}"
	@echo "${shell ${AR} --version | head -n 1}"

	@echo "------------------------ ${BLUE} BUILD ${NC} ------------------------"
	@mkdir -p ${BUILD_DIR}
	@${MAKE} kernel

	@echo "------------------------ ${GREEN} DEBUG ${NC} ------------------------"
	@${MAKE} debug

	@echo "Build ${GREEN}complete${NC}. Enjoy life <3"

run:
	${info Running ${PROJECT_NAME} for ${TARGET_ARCH}}

	@${QEMU_SCRIPT} -nographic -no-reboot -kernel ${BUILD_DIR}/kernel.elf

clean:
	@echo "Cleaning all build files under ${BUILD_DIR}"
	@find ${BUILD_DIR} -name "*.o" -type f -delete
	@find ${BUILD_DIR} -name "*.elf" -type f -delete

	@echo "Cleaning all debug files under ${BUILD_DIR}"
	@find ${BUILD_DIR} -name "*.asm" -type f -delete
	@find ${BUILD_DIR} -name "*.sym" -type f -delete
