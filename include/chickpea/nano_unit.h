#ifndef CHICKPEA_NANO_UNIT_H
#define CHICKPEA_NANO_UNIT_H

/**
 * A microscopic unit testing framework that should run on actual GBA
 * hardware as well as emulated. Lightly inspired by the Linux kernel's kunit
 * testing framework, except even more minimal.
 *
 * On non-GBA platforms, it prints results in TAP (Test Anything Protocol)
 * format to stdout.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Define a nano_unit_case based on a test case function.
 *
 * These should go together in an array that's terminated by an empty
 * nano_unit_case
 */
#define NANO_UNIT_CASE(test)                                \
	{                                                   \
		.run = test, .name = #test, .success = true \
	}

/**
 * Define a nano_unit_suite based on an array of test cases.
 *
 * These should also be defined together in an array that's terminated by an
 * empty nano_unit_suite.
 */
#define NANO_UNIT_SUITE(test_cases)                                       \
	{                                                                 \
		.cases = test_cases, .name = #test_cases, .success = true \
	}

struct nano_unit_case {
	void (*run)(struct nano_unit_case *test);
	const char *name;
	const char *msg;
	bool success;
};

struct nano_unit_suite {
	struct nano_unit_case *cases;
	const char *name;
	bool success;
};

/**
 * Run all test cases in all test suites
 *
 * @param suites An array of nano_unit_suites that end in an empty
 * nano_unit_suite.
 */
void nano_unit_run_suites(struct nano_unit_suite *suites);

/**
 * Instead of implementing some sort of try catch, or returning implicitly,
 * NANO_ASSERT() takes a label as the last argument which will be jumped to if
 * the assertion fails. This will allow you to do anything clean up that will
 * need to happen within the test case itself.
 */

#define NANO_ASSERT(test, expr, exit)          \
	do {                                   \
		if (!(expr)) {                 \
			test->success = false; \
			test->msg = #expr;     \
			goto exit;             \
		}                              \
	} while (0)

#endif //CHICKPEA_NANO_UNIT_H