/*
 * FileVault v2.0 — Test Framework
 * Minimal C test macros — no external dependencies required.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        tests_run++; \
        if (!(cond)) { \
            tests_failed++; \
            fprintf(stderr, "  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
        } else { \
            tests_passed++; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(a, b, msg) \
    TEST_ASSERT((a) == (b), (msg))

#define TEST_ASSERT_NEQ(a, b, msg) \
    TEST_ASSERT((a) != (b), (msg))

#define TEST_ASSERT_STR_EQ(a, b, msg) \
    TEST_ASSERT(strcmp((a), (b)) == 0, (msg))

#define TEST_RUN(test_fn) \
    do { \
        printf("  Running: %s\n", #test_fn); \
        test_fn(); \
    } while(0)

#define TEST_SUMMARY() \
    do { \
        printf("\n  ────────────────────────────────────\n"); \
        printf("  Tests run:    %d\n", tests_run); \
        printf("  Tests passed: %d\n", tests_passed); \
        printf("  Tests failed: %d\n", tests_failed); \
        printf("  ────────────────────────────────────\n\n"); \
    } while(0)

#define TEST_EXIT() \
    return (tests_failed > 0) ? 1 : 0

#endif /* TEST_FRAMEWORK_H */
