#include "chickpea/nano_unit.h"

#include <stdio.h>

static uint32_t count_tests_in_suites(struct nano_unit_suite *suites)
{
	uint32_t tests = 0;
	for (struct nano_unit_suite *suite = suites; suite->cases; suite++) {
		for (struct nano_unit_case *test_case = suite->cases;
		     test_case->run; test_case++) {
			tests++;
		}
	}
	return tests;
}

static void run_tests_in_suite(struct nano_unit_suite *suite, uint32_t *count)
{
	for (struct nano_unit_case *test_case = suite->cases; test_case->run;
	     test_case++) {
		*count += 1;
		test_case->run(test_case);
		const char *msg = test_case->success ? "ok" : "not ok";
		if (test_case->msg) {
			printf("%s %i - %s::%s (failed on \"%s\")\n", msg,
			       *count, suite->name, test_case->name,
			       test_case->msg);
		} else {
			printf("%s %i - %s::%s\n", msg, *count, suite->name,
			       test_case->name);
		}
		suite->success = suite->success && test_case->success;
	}
}

void nano_unit_run_suites(struct nano_unit_suite *suites)
{
	uint32_t num_tests = count_tests_in_suites(suites);
	printf("1..%i\n", num_tests);

	uint32_t count = 0;
	for (struct nano_unit_suite *suite = suites; suite->cases; suite++) {
		run_tests_in_suite(suite, &count);
	}
}