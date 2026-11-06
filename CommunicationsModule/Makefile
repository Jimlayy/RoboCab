CC       ?= gcc
CFLAGS   ?= -std=gnu11 -Wall -Wextra -O2 -g
CFLAGS   += -pthread
CPPFLAGS += -Iinclude
LDLIBS   += -pthread

BUILD  := build
TARGET := $(BUILD)/communications_module

COMMON := ../Common

SRCS := $(wildcard src/*.c) $(wildcard $(COMMON)/*.c)
OBJS := $(patsubst %.c,$(BUILD)/%.o,$(notdir $(SRCS)))

# Where make looks for the .c files named in OBJS
vpath %.c src $(COMMON)

# Tests and tools link every component except main().
LIB_OBJS := $(filter-out $(BUILD)/main.o,$(OBJS))
TESTS    := $(patsubst tests/%.c,$(BUILD)/tests/%,$(wildcard tests/*.c))
TOOLS    := $(patsubst tools/%.c,$(BUILD)/tools/%,$(wildcard tools/*.c))

.PHONY: all run test tools clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/tests/%: tests/%.c $(LIB_OBJS) | $(BUILD)/tests
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/tools/%: tools/%.c $(LIB_OBJS) | $(BUILD)/tools
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD) $(BUILD)/tests $(BUILD)/tools:
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

tools: $(TOOLS)

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)
