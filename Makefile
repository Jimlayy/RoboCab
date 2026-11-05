# make avr   kompilerar logiken för ATmega1284
# make clean tar bort byggfiler

CC ?= gcc
AVR_CC := avr-gcc
MCU := atmega1284

CFLAGS := -std=c11 -Wall -Wextra -Werror -g $(addprefix -I,$(wildcard */logic))

LOGIC_HDRS := $(wildcard */logic/*.h)

# Bara det som körs på en ATmega. Kommunikations- och kameramodulen körs på Pi:n.
AVR_SRCS := $(wildcard Common/logic/*.c SteeringModule/logic/*.c SensorModule/logic/*.c)
AVR_OBJS := $(patsubst %.c,build/avr/%.o,$(AVR_SRCS))

.PHONY: avr clean

avr: $(AVR_OBJS)

build/avr/%.o: %.c $(LOGIC_HDRS)
	@mkdir -p $(@D)
	$(AVR_CC) $(CFLAGS) -mmcu=$(MCU) -Os -c $< -o $@

clean:
	rm -rf build
