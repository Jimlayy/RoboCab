MCU=atmega328
F_CPU=16000000

CC=avr-gcc
OBJCOPY=avr-objcopy
TARGET=main

# Directories
SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build

# Added -I${INC_DIR} so the compiler knows where to look for your headers
CFLAGS=-std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I${INC_DIR}

# Find all .c files in src/ and map their object files to build/
SRCS=$(wildcard ${SRC_DIR}/*.c)
OBJS=$(patsubst ${SRC_DIR}/%.c, ${BUILD_DIR}/%.o, ${SRCS})

all: ${BUILD_DIR}/${TARGET}.hex

# Create the build directory if it doesn't exist
${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

${BUILD_DIR}/${TARGET}.hex: ${BUILD_DIR}/${TARGET}.elf
	${OBJCOPY} -j .text -j .data -O ihex $< $@

${BUILD_DIR}/${TARGET}.elf: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^

# The pipe '|' ensures the build directory is created BEFORE compiling
${BUILD_DIR}/%.o: ${SRC_DIR}/%.c | ${BUILD_DIR}
	${CC} ${CFLAGS} -c $< -o $@

# Clean now just deletes the entire build folder
clean: 
	rm -rf ${BUILD_DIR}