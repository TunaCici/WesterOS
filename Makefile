#
# Main Makefile
#
# Author: Tuna CICI
#

# Colored printing
RED 	= \033[0;31m
GREEN 	= \033[0;32m
YELLOW 	= \033[0;33m
BLUE 	= \033[0;34m
MAGENTA = \033[0;35m
CYAN 	= \033[0;36m
NC	= \033[0m

# Project
PROJECT_NAME 	= WesterOS
PROJECT_DIR 	= ${shell pwd}
BUILD_DIR 	= Build
TEST_DIR	= Tests

# Target architecture & Toolchain
TARGET_ARCH = aarch64
TOOLCHAIN_PATH = Toolchain/arm-gnu-toolchain-13.2.Rel1-darwin-arm64-aarch64-none-elf

# Cross-compiler
CC = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc
CXX = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++
AS = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-as
LD = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld
AR = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ar
OBJCOPY = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-objcopy
OBJDUMP = ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-objdump

# Host-compiler
HOST_CC = clang
HOST_CXX = clang++
HOST_AR = ar

# Flags
INCLUDES = \
	-I Kernel/Include -I Kernel/Arch \
	-I Tests/googletest/googletest/include
CCFLAGS = ${INCLUDES} -march=armv8-a -mtune=cortex-a72 -mno-outline-atomics -g \
	-Wall -Wextra -ffreestanding -nostdlib -std=gnu99 -DDEBUG
CXXFLAGS = ${INCLUDES} -march=armv8-a -mtune=cortex-a72 -g \
	-Wall -Wextra -ffreestanding -nostdlib -std=c++20 -DDEBUG
HOST_CCFLAGS = ${INCLUDES} -Wall -Wextra -std=gnu99 -g -m64
HOST_CXXFLAGS = ${INCLUDES} -Wall -Wextra -std=c++20 -g -m64

# QEMU
QEMU_SCRIPT = Emulation/launch-qemu.sh

# GoogleTest
GTEST_DIR = Tests/googletest/googletest
GTEST_HEADERS = ${GTEST_DIR}/include/gtest/*.h \
                ${GTEST_DIR}/include/gtest/internal/*.h
GTEST_SRCS = ${GTEST_DIR}/src/*.cc ${GTEST_DIR}/src/*.h ${GTEST_HEADERS}
GTEST_LIBS = libgtest.a libgtest_main.a

GTEST_CPPFLAGS = ${INCLUDES} -g -isystem ${GTEST_DIR}/include
GTEST_CXXFLAGS = ${INCLUDES} -g -Wall -Wextra -std=c++20 \
	-Wno-unused-command-line-argument \
	-dead_strip \
# Flag: ^^^^^^^^^^^ removes "unused symbols". Not optimal, but it works >.<
# Without it I'd get errors like: undefined symbol "_kernel_pgtbl".
# The files like Kernel/Main.c uses symbols defined in Kernel/Kernel.ld
# When compiling tests ALL objects are are given to the clang/ld.
# Since Start.c and Main.c will [probably] never gonna get unit tested
# We can just ignore "unreslved symbols" with -dead-strip.
# > maybe in the future I can find a better way. who knows...

# Project source files
SRCS = \
	Kernel/Arch/ARM64/Start.c \
	Kernel/Arch/ARM64/Exception.c \
	Kernel/Main.c \
	Kernel/Library/LibKern/DeviceTree.c \
	Kernel/Library/LibKern/Console.c \
	Kernel/Library/LibKern/Time.c \
	Kernel/Memory/BootMem.c \
	Kernel/Memory/Physical.c \
	Kernel/Memory/Virtual.c
OBJS = ${SRCS:.c=.o}

ASMS = \
	Kernel/Arch/ARM64/Entry.S \
	Kernel/Arch/ARM64/Vector.S \
	Kernel/Library/LibKern/String/memchr.S \
	Kernel/Library/LibKern/String/memcmp.S \
	Kernel/Library/LibKern/String/memcpy.S \
	Kernel/Library/LibKern/String/memrchr.S \
	Kernel/Library/LibKern/String/memset.S \
	Kernel/Library/LibKern/String/strchr.S \
	Kernel/Library/LibKern/String/strcmp.S \
	Kernel/Library/LibKern/String/strcpy.S \
	Kernel/Library/LibKern/String/strlen.S \
	Kernel/Library/LibKern/String/strncmp.S \
	Kernel/Library/LibKern/String/strnlen.S \
	Kernel/Library/LibKern/String/strrchr.S
ASM_OBJS = ${ASMS:.S=.o}

# Test source files (must be hardware-independent)
TEST_SRCS = \
	Tests/PageDefTest.cpp \
	Tests/BootMemTest.cpp \
	Tests/PhysicalTest.cpp \
	Kernel/Memory/BootMem.c \
	Kernel/Memory/Physical.c
TEST_OBJS := ${filter %.o, ${TEST_SRCS:.c=.o}}
TEST_OBJS += ${filter %.o, ${TEST_SRCS:.cpp=.o}}

LDSCRIPT = Kernel/kernel.ld

# To switch between CC and HOST_CC (helpful when compiling tests)
CROSS ?= True

default: all

%.o: %.S
	@echo "CC $<"
	@${CC} ${CCFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"

%.o: %.c
ifeq (${CROSS}, True)
	@echo "CC $<"
	@${CC} ${CCFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"
else
	@echo "HOST_CC $<"
	@${HOST_CC} ${HOST_CCFLAGS} -c $< -o ${TEST_DIR}/${notdir $@}
	@echo "HOST_CC $< ${GREEN}ok${NC}"
endif

%.o: %.cpp
ifeq (${CROSS}, True)
	@echo "CXX $<"
	@${CXX} ${CXXFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CXX $< ${GREEN}ok${NC}"
else
	@echo "HOST_CXX $<"
	@${HOST_CXX} ${HOST_CXXFLAGS} -c $< -o ${TEST_DIR}/${notdir $@}
	@echo "HOST_CXX $< ${GREEN}ok${NC}"
endif

kernel: ${ASM_OBJS} ${OBJS}
	@echo "LINK kernel.elf -T ${LDSCRIPT}"
	@${LD} -T ${LDSCRIPT} -o ${BUILD_DIR}/kernel.elf ${addprefix ${BUILD_DIR}/, $(notdir ${ASM_OBJS})} ${addprefix ${BUILD_DIR}/, $(notdir ${OBJS})}
	@echo "LINK kernel.elf ${GREEN}ok${NC}"

debug:
	@echo "OBJDUMP ${BUILD_DIR}/kernel.elf"
	@${OBJDUMP} -S ${BUILD_DIR}/kernel.elf > ${BUILD_DIR}/kernel.asm
	@${OBJDUMP} -t ${BUILD_DIR}/kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > ${BUILD_DIR}/kernel.sym
	@echo "OBJDUMP ${BUILD_DIR}/kernel.elf ${GREEN}ok${NC}"

	@echo "OBJDUMP ${BUILD_DIR}/Entry.o"
	@${OBJDUMP} -S ${BUILD_DIR}/Entry.o > ${BUILD_DIR}/entry.asm
	@echo "OBJDUMP ${BUILD_DIR}/Entry.o ${GREEN}ok${NC}"

	@echo "OBJDUMP ${BUILD_DIR}/Vector.o"
	@${OBJDUMP} -S ${BUILD_DIR}/Vector.o > ${BUILD_DIR}/vector.asm
	@echo "OBJDUMP ${BUILD_DIR}/Vector.o ${GREEN}ok${NC}"

compiledb:
	@echo "COMPILEDB -n make all"
	@compiledb -n make all
	@echo "COMPILEDB -n make all ${GREEN}ok${NC}"

all:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "${shell ${CC} --version | head -n 1}"
	@echo "${shell ${CXX} --version | head -n 1}"
	@echo "${shell ${LD} --version | head -n 1}"
	@echo "${shell ${AR} --version | head -n 1}"

	@echo "--------------------- ${BLUE} BUILD KERNEL ${NC} ---------------------"
	@mkdir -p ${BUILD_DIR}
	@${MAKE} kernel

	@echo "--------------------- ${BLUE} BUILD TESTS ${NC} ----------------------"
	@${MAKE} all_test CROSS=False

	@echo "------------------------ ${GREEN} DEBUG ${NC} ------------------------"
	@${MAKE} debug

	@echo "---------------------------------------------------------"
	@echo "Build ${GREEN}complete${NC}. Enjoy life <3"

run:
	${info Running ${PROJECT_NAME} for ${TARGET_ARCH}}
	@${QEMU_SCRIPT} -d int -nographic -no-reboot -kernel ${BUILD_DIR}/kernel.elf

run-debug:
	${info Debugging ${PROJECT_NAME} for ${TARGET_ARCH}}
	@${QEMU_SCRIPT} -s -S -d int -nographic -no-reboot -kernel ${BUILD_DIR}/kernel.elf

# GoogleTest libraries
libgtest.a: ${GTEST_SRCS}
	@echo "HOST_CXX $<"
	@${HOST_CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} -I${GTEST_DIR} -c \
            ${GTEST_DIR}/src/gtest-all.cc -o ${TEST_DIR}/gtest-all.o
	@echo "HOST_CXX $< ${GREEN}ok${NC}"

	@echo "HOST_AR ${TEST_DIR}/$@"	
	@${HOST_AR} ${ARFLAGS} ${TEST_DIR}/$@ \
		${TEST_DIR}/gtest-all.o
	@echo "HOST_AR ${TEST_DIR}/$@ ${GREEN}ok${NC}"

libgtest_main.a: libgtest.a ${GTEST_SRCS}
	@echo "HOST_CXX src/gtest_main.cc"
	@${HOST_CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} -I${GTEST_DIR} -c \
            ${GTEST_DIR}/src/gtest_main.cc -o ${TEST_DIR}/gtest_main.o
	@echo "HOST_CXX src/gtest_main.cc ${GREEN}ok${NC}"

	@echo "HOST_AR ${TEST_DIR}/$@"	
	@${HOST_AR} ${ARFLAGS} ${TEST_DIR}/$@ \
		${TEST_DIR}/gtest-all.o ${TEST_DIR}/gtest_main.o
	@echo "HOST_AR ${TEST_DIR}/$@ ${GREEN}ok${NC}"

all_test: ${TEST_OBJS} ${GTEST_LIBS}
	@echo "HOST_CXX ${addprefix ${TEST_DIR}/, $(notdir ${TEST_OBJS})} ${TEST_DIR}/libgtest_main.a"
	@${HOST_CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} \
		${addprefix ${TEST_DIR}/, $(notdir ${TEST_OBJS})} \
		${TEST_DIR}/libgtest_main.a -o ${TEST_DIR}/All_Test
	@echo "HOST_CXX ${TEST_OBJS} ${addprefix ${TEST_DIR}/, $(notdir ${OBJS})} ${TEST_DIR}/libgtest_main.a ${GREEN}ok${NC}"

test:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "${shell ${HOST_CC} --version | head -n 1}"
	@echo "${shell ${HOST_CXX} --version | head -n 1}"

	@echo "------------------------ ${YELLOW} CLEAN ${NC} ------------------------"
	@echo "Deleting all object files (*.o)"
	@find ${TEST_DIR} -name "*.o" -type f -delete

	@echo "------------------------ ${BLUE} BUILD ${NC} ------------------------"
	@${MAKE} all_test CROSS=False

	@echo "------------------------ ${GREEN} TEST ${NC} ------------------------"
	@${TEST_DIR}/All_Test

clean:
	@echo "Cleaning all build files under ${BUILD_DIR}"
	@find ${BUILD_DIR} -name "*.o" -type f -delete
	@find ${BUILD_DIR} -name "*.elf" -type f -delete

	@echo "Cleaning all debug files under ${BUILD_DIR}"
	@find ${BUILD_DIR} -name "*.asm" -type f -delete
	@find ${BUILD_DIR} -name "*.sym" -type f -delete

	@echo "Cleaning all test files under ${TEST_DIR}"
	@find ${TEST_DIR} -name "*.o" -type f -delete
	@find ${TEST_DIR} -name "*.a" -type f -delete
	@find ${TEST_DIR} -name "${TEST_OBJS}" -type f -delete
	@find ${TEST_DIR} -name "All_Test" -type f -delete

	@echo "Cleaning 'compile_commands.json' ."
	@find . -name "compile_commands.json" -type f -delete
