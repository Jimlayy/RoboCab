# make test  kör alla tester på den här datorn
# make avr   kompilerar logiken för ATmega1284
# make clean tar bort byggfiler
#
# Kod i <Modul>/logic/ får inte röra hårdvara, så att den går att testa här.

CC ?= gcc
AVR_CC := avr-gcc
MCU := atmega1284

CFLAGS := -std=c11 -Wall -Wextra -Werror -g $(addprefix -I,$(wildcard */logic))
SANITIZE := -fsanitize=address,undefined -fno-sanitize-recover=all
# Läckagekontrollen fungerar inte alltid i Docker, och vi använder inte malloc.
export ASAN_OPTIONS := detect_leaks=0

LOGIC_SRCS := $(wildcard */logic/*.c)
LOGIC_HDRS := $(wildcard */logic/*.h)
TESTS := $(patsubst Tests/%.c,build/%,$(wildcard Tests/test_*.c))

# Bara det som körs på en ATmega. Kommunikations- och kameramodulen körs på Pi:n.
AVR_SRCS := $(wildcard Common/logic/*.c SteeringModule/logic/*.c SensorModule/logic/*.c)
AVR_OBJS := $(patsubst %.c,build/avr/%.o,$(AVR_SRCS))

.PHONY: test avr clean

test: $(TESTS)
	@status=0; for t in $(TESTS); do ./$$t || status=1; done; exit $$status

build/%: Tests/%.c Tests/test.h $(LOGIC_SRCS) $(LOGIC_HDRS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(SANITIZE) $< $(LOGIC_SRCS) -o $@

avr: $(AVR_OBJS)

build/avr/%.o: %.c $(LOGIC_HDRS)
	@mkdir -p $(@D)
	$(AVR_CC) $(CFLAGS) -mmcu=$(MCU) -Os -c $< -o $@

clean:
	rm -rf build
