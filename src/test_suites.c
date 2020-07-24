#include "chickpea/nano_unit.h"

extern struct nano_unit_case common_test_suite[];
extern struct nano_unit_case bit_vec_test_suite[];
extern struct nano_unit_case obj_tiles_test_suite[];

struct nano_unit_suite test_suites[] = { NANO_UNIT_SUITE(common_test_suite),
					 NANO_UNIT_SUITE(bit_vec_test_suite),
					 NANO_UNIT_SUITE(obj_tiles_test_suite),
					 {} };
