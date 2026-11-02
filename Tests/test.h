#ifndef TEST_H
#define TEST_H

#include <stdio.h>

static int test_failures;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            printf("  %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
            test_failures++;                                            \
        }                                                               \
    } while (0)

#define CHECK_EQ(actual, expected)                                      \
    do {                                                                \
        long long a_ = (actual);                                        \
        long long e_ = (expected);                                      \
        if (a_ != e_) {                                                 \
            printf("  %s:%d: %s blev %lld, väntade %lld\n",             \
                   __FILE__, __LINE__, #actual, a_, e_);                \
            test_failures++;                                            \
        }                                                               \
    } while (0)

#define RUN(test)                                                       \
    do {                                                                \
        int before_ = test_failures;                                    \
        test();                                                         \
        printf("%s %s\n", before_ == test_failures ? "OK " : "FEL", #test); \
    } while (0)

#define TEST_RESULT() (test_failures ? 1 : 0)

#endif
