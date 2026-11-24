# Testing

This guide describes how the automated tests in RoboCab are organised, how to run
them and how to write new ones. Automated tests check the logic. Hardware, timing
and complete runs on the track are tested manually and recorded in the test protocol.

## Overview

| What | Framework | Where | Run with |
|---|---|---|---|
| C code, all modules | Unity v2.7.0 (in `Vendor/unity/`) | `<Module>/tests/` | `make c-test` |
| C code, one module | Unity v2.7.0 (in `Vendor/unity/`) | `<Module>/tests/` | `make common-test`, `make comms-test`, `make steering-test` or `make sensor-test` |
| Camera module (Python) | pytest | `CameraModule/tests/` | `make camera-test` |

Every push to GitLab runs the pipeline. A red pipeline means that something no
longer builds or that a test fails. Do not merge a branch with a red pipeline.

## Running the tests locally

Run all commands from the root of the repository.

**Windows:** use Ubuntu in WSL. Git Bash does not have `gcc` or `make`.
The repository on `C:` is found under `/mnt/c/...`.

```
sudo apt install build-essential python3-opencv python3-pytest
```

**Mac:** install the command line tools with `xcode-select --install`.
For the camera tests, install `opencv-python-headless`, `numpy` and `pytest` with pip,
for example in a virtual environment.

Make rebuilds a test automatically when the test or the code it tests has changed.

## What can be tested

Only code that does not depend on hardware can be tested automatically: calculations,
decisions, state machines and packing or unpacking of data.

* Code that uses `<avr/io.h>`, registers, interrupts or real devices cannot run on a PC.
* For the steering and sensor modules, testable code goes in `<Module>/logic/`.
  Files there must not include `<avr/io.h>`.
* If a function mixes logic and hardware, split it: a pure function that calculates
  the value, and a thin function that writes it to the register.
* The communications module runs on the Raspberry Pi (Linux), so its code in `src/`
  can be tested directly, except code that opens ports.

## C tests

### Where the files go

| Module | Tests | Code under test |
|---|---|---|
| Common | `Common/tests/` | `Common/logic/` |
| Communications module | `CommunicationsModule/tests/` | `CommunicationsModule/src/` |
| Steering module | `SteeringModule/tests/` | `SteeringModule/logic/` |
| Sensor module | `SensorModule/tests/` | `SensorModule/logic/` |

### Naming rules

* A test file tests exactly one C file and is named `test_<filename>.c`.
  For example, `Common/tests/test_parser.c` tests `Common/logic/parser.c`.
* Make finds new test files automatically through this rule. No change in the
  Makefile is needed. A test file without a matching C file gives the error
  `No rule to make target ...`.
* Test functions start with `test_` and describe the behaviour, for example
  `test_bad_checksum_is_dropped`.

### Structure of a test file

```c
#include "unity.h"
#include "<filename>.h"

void setUp(void)        /* runs before every test */
{
}

void tearDown(void)     /* runs after every test */
{
}

void test_something_happens(void)
{
    /* Arrange: set up input */
    /* Act: call the function */
    /* Assert: compare with the expected result */
    TEST_ASSERT_EQUAL_INT(1, result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_something_happens);
    return UNITY_END();
}
```

* `setUp` and `tearDown` must exist, even if they are empty. Reset all shared
  variables in `setUp` so that the tests do not affect each other.
* A test that is not listed with `RUN_TEST` in `main` is never run.
* In `TEST_ASSERT_EQUAL_...` the expected value comes first and the actual value second.

### Useful assertions

| Assertion | Use |
|---|---|
| `TEST_ASSERT_EQUAL_INT(expected, actual)` | Integers |
| `TEST_ASSERT_EQUAL_UINT8(expected, actual)` | Bytes |
| `TEST_ASSERT_EQUAL_INT16(expected, actual)` | 16-bit values, for example return codes |
| `TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, n)` | `n` bytes at once |
| `TEST_ASSERT_TRUE(condition)`, `TEST_ASSERT_FALSE(condition)` | Conditions |
| `TEST_IGNORE_MESSAGE("reason")` | Marks a test that waits for a decision. It shows as IGNORED and does not make the pipeline red. |

### Tests that need more C files

If a test needs more C files than the one it tests, add a line to the Makefile, for example:

```
build/comms/test_packet_handler: CommunicationsModule/src/sensor_data.c
```

## Python tests (camera module)

* Tests are in `CameraModule/tests/` and are named `test_<filename>.py`.
* Image processing that can be tested goes in `vision.py`. `camera.py` only starts
  the camera and calls `vision.py`.
* Use `pytest.mark.parametrize` for cases with the same shape, and give each case an `id`
  so that a failing case is easy to find.
* Run a single test with `python3 -m pytest tests -k <name>` from `CameraModule/`.

## How to choose test cases

1. **Equivalence classes:** divide the input into groups that should behave the same,
   and test at least one value from each group.
2. **Boundary values:** test exactly at each boundary and one step on each side,
   for example length 0, 1, max and max + 1.
3. **Errors and recovery:** invalid input, and that the code works again afterwards.
4. **One test per found bug:** write a test that shows the bug before fixing it.

Calculate expected values by hand or from the specification, never by running the code.

## Good practice

* One behaviour per test. A failing test should point to one problem.
* Tests must be independent and give the same result every time:
  no randomness, clocks, hardware or network.
* Write the tests in the same merge request as the code.
* If behaviour changes on purpose, change the test in the same commit and explain why.
* Never remove or weaken a test only to make the pipeline green.

## CI

| Job | Does |
|---|---|
| `build-avr` | Checks that the logic builds for the ATmega |
| `c-tests` | Runs `make c-test` |
| `camera-tests` | Runs `make camera-test` |

* The pipeline runs in Linux.
* Folders that only exist on your computer, such as `build/`, do not exist in the
  pipeline.
* If a job is stuck in "pending", no runner is available. This is not a code error.
* The log of a failed job shows the file, line, test and expected and actual value.

## When a test fails

1. Read the log in GitLab under **Build → Pipelines**.
2. Run the same `make` command locally.
3. Decide whether the code or the test is wrong.
4. Fix it, push to the same branch and check that the pipeline turns green.