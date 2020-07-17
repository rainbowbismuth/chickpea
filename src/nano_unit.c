#include "chickpea/common.h"
#include "chickpea/nano_unit.h"

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

		debug_put_str(test_case->success ? "ok " : "not ok ");
		debug_put_u32(*count);
		debug_put_str(" - ");
		debug_put_str(suite->name);
		debug_put_str("::");
		debug_put_str(test_case->name);
		if (test_case->msg) {
			debug_put_str(" (failed on\"");
			debug_put_str(test_case->msg);
			debug_put_str("\")\n");
		} else {
			debug_put_str("\n");
		}

		suite->success = suite->success && test_case->success;
	}
}

void nano_unit_run_suites(struct nano_unit_suite *suites)
{
	uint32_t num_tests = count_tests_in_suites(suites);
	debug_put_str("1..");
	debug_put_u32(num_tests);
	debug_put_char('\n');

	uint32_t count = 0;
	for (struct nano_unit_suite *suite = suites; suite->cases; suite++) {
		run_tests_in_suite(suite, &count);
	}
}