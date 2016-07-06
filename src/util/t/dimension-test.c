#include "conf.h"

#include <stdlib.h>
#include <string.h>

#include <CUnit/Basic.h>

#include "misc.h"

#define STR_A "22px 100%"
#define STR_B "22px 1024px"
#define STR_C "5%            100%"

char* str_a;
char* str_b;
char* str_c;

dimension_t dimen_a;
dimension_t dimen_b;
dimension_t dimen_c;

int
dimension_setup_suite (void) {
  str_a = strdup (STR_A);
  str_b = strdup (STR_B);
  str_c = strdup (STR_C);

  return 0;
}

int
dimension_teardown_suite (void) {
  free (str_a);
  free (str_b);
  free (str_c);

  return 0;
}

void test_dimension_create_from_str (void) {
  dimen_a = dimension_create_from_str (str_a);
  dimen_b = dimension_create_from_str (str_b);
  dimen_c = dimension_create_from_str (str_c);

  CU_ASSERT (dimen_a.width.unit == Pixels);
  CU_ASSERT (dimen_a.height.unit == Percentage);
  CU_ASSERT (dimen_a.width.value != 0.0);
  CU_ASSERT (dimen_a.width.value == 22.0);
  CU_ASSERT (dimen_a.height.value != 0.0);

  CU_ASSERT (dimen_b.width.unit == Pixels);
  CU_ASSERT (dimen_b.height.unit == Pixels);
  CU_ASSERT (dimen_b.width.value != 0.0);
  CU_ASSERT (dimen_b.height.value != 0.0);

  CU_ASSERT (dimen_c.width.unit == Percentage);
  CU_ASSERT (dimen_c.height.unit == Percentage);
  CU_ASSERT (dimen_c.width.value != 0.0);
  CU_ASSERT (dimen_c.height.value != 0.0);
}

void
test_dimension (void) {

  CU_pSuite suite = CU_add_suite ("dimension",
				  dimension_setup_suite,
				  dimension_teardown_suite);

  if (!suite) return;

  if (!CU_add_test (suite, "dimension/create-from-str",
		    test_dimension_create_from_str))
    return;
}
