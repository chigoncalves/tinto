#include "conf.h"

#include <stdlib.h>

#include <CUnit/Basic.h>

#include "desktop-entry-test.h"
#include "color32-test.h"
#include "path-utils-test.h"
#include "string-addins-test.h"
#include "dimension-test.h"

int
main (void) {
  if (CU_initialize_registry () != CUE_SUCCESS)
    return CU_get_error ();

  test_desktop_entry ();
  test_color32 ();
  test_path_utils ();
  test_string_addins ();
  test_dimension ();

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return EXIT_SUCCESS;
}
