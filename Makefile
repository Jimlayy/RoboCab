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

.PHONY: avr camera-test common-test comms-test steering-test sensor-test c-test clean

avr: $(AVR_OBJS)

build/avr/%.o: %.c $(LOGIC_HDRS)
	@mkdir -p $(@D)
	$(AVR_CC) $(CFLAGS) -mmcu=$(MCU) -Os -c $< -o $@

camera-test:
	cd CameraModule && python3 -m pytest tests -v


# ===================== C tests (Unity) =====================
# Every module follows the same convention:
#   <Module>/tests/test_<name>.c tests <code folder>/<name>.c
# A new test file is picked up automatically. Run one module with
# make <module>-test, or every C test with make c-test.

UNITY := Vendor/unity
TEST_CFLAGS := -std=c11 -Wall -Wextra -I$(UNITY)

# Runs every test program given as a prerequisite, even if one fails.
RUN_TESTS = @status=0; for t in $^; do ./$$t || status=1; done; exit $$status

# ---- Common: tests in Common/tests, code in Common/Logic ----
COMMON_TESTS := $(patsubst Common/tests/%.c,build/common/%,$(wildcard Common/tests/test_*.c))

build/common/test_%: Common/tests/test_%.c Common/Logic/%.c $(UNITY)/unity.c
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -ICommon/Logic $^ -o $@

common-test: $(COMMON_TESTS)
	$(RUN_TESTS)

# ---- Communications module: tests in CommunicationsModule/tests, code in src/ ----
COMMS_TESTS := $(patsubst CommunicationsModule/tests/%.c,build/comms/%,$(wildcard CommunicationsModule/tests/test_*.c))

build/comms/test_%: CommunicationsModule/tests/test_%.c CommunicationsModule/src/%.c $(UNITY)/unity.c
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -pthread -ICommunicationsModule/include -ICommon/Logic $^ -o $@

comms-test: $(COMMS_TESTS)
	$(RUN_TESTS)

# ---- Steering module: tests in SteeringModule/tests, code in SteeringModule/logic ----
# Only code without <avr/io.h> can be tested here, so it goes in logic/.
STEERING_TESTS := $(patsubst SteeringModule/tests/%.c,build/steering/%,$(wildcard SteeringModule/tests/test_*.c))

build/steering/test_%: SteeringModule/tests/test_%.c SteeringModule/logic/%.c $(UNITY)/unity.c
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -ISteeringModule/logic -ICommon/Logic $^ -o $@

steering-test: $(STEERING_TESTS)
	$(RUN_TESTS)

# ---- Sensor module: tests in SensorModule/tests, code in SensorModule/logic ----
# Only code without <avr/io.h> can be tested here, so it goes in logic/.
SENSOR_TESTS := $(patsubst SensorModule/tests/%.c,build/sensor/%,$(wildcard SensorModule/tests/test_*.c))

build/sensor/test_%: SensorModule/tests/test_%.c SensorModule/logic/%.c $(UNITY)/unity.c
	@mkdir -p $(@D)
	$(CC) $(TEST_CFLAGS) -ISensorModule/logic -ICommon/Logic $^ -o $@

sensor-test: $(SENSOR_TESTS)
	$(RUN_TESTS)

# ---- All C tests ----
C_TESTS := $(COMMON_TESTS) $(COMMS_TESTS) $(STEERING_TESTS) $(SENSOR_TESTS)

c-test: $(C_TESTS)
	$(RUN_TESTS)


clean:
	rm -rf build
