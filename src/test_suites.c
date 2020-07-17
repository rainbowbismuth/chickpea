#include "chickpea/nano_unit.h"

extern struct nano_unit_case common_test_suite[];

struct nano_unit_suite test_suites[] = {
	NANO_UNIT_SUITE(common_test_suite),
	{}
};
