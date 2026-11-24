MCU=atmega1284p
F_CPU=1000000UL

CC=avr-gcc
OBJCOPY=avr-objcopy
TARGET=main

# Directories
SRC_DIR1 = src-1284
SRC_DIR2 = ../Common/Logic
INC_DIR1 = inc
INC_DIR2 = ../Common/Logic
BUILD_DIR = build-1284

# Added -I${INC_DIR2} so the compiler finds the headers in the Common logic folder
CFLAGS=-std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I${INC_DIR1} -I${INC_DIR2}

# Find all .c files in both directories
SRCS1=$(wildcard ${SRC_DIR1}/*.c)
SRCS2=$(wildcard ${SRC_DIR2}/*.c)

# Map their object files to the build directory
OBJS1=$(patsubst ${SRC_DIR1}/%.c, ${BUILD_DIR}/%.o, ${SRCS1})
OBJS2=$(patsubst ${SRC_DIR2}/%.c, ${BUILD_DIR}/%.o, ${SRCS2})

# Combine all object files
OBJS=${OBJS1} ${OBJS2}

all: ${BUILD_DIR}/${TARGET}.hex

# Create the build directory if it doesn't exist
${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

${BUILD_DIR}/${TARGET}.hex: ${BUILD_DIR}/${TARGET}.elf
	${OBJCOPY} -j .text -j .data -O ihex $< $@

${BUILD_DIR}/${TARGET}.elf: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^

# Rule for compiling sources from SRC_DIR1
${BUILD_DIR}/%.o: ${SRC_DIR1}/%.c | ${BUILD_DIR}
	${CC} ${CFLAGS} -c $< -o $@

# Rule for compiling sources from SRC_DIR2
${BUILD_DIR}/%.o: ${SRC_DIR2}/%.c | ${BUILD_DIR}
	${CC} ${CFLAGS} -c $< -o $@

# Clean now just deletes the entire build folder
clean: 
	rm -rf ${BUILD_DIR}