#include "conf.h"

#include <stdlib.h> // For `free', `getenv'.
#include <string.h> // For `strdup'.

#include <CUnit/Basic.h>


#include "debug.h"
#include "path-utils.h"

#define TEST_PATH_A "~"
#define EXPECTED_PATH_A "/home/crowseye"
#define TEST_PATH_B "~/"
#define EXPECTED_PATH_B "/home/crowseye/"
#define TEST_PATH_C "~/foo"
#define EXPECTED_PATH_C "/home/crowseye/foo"
#define TEST_PATH_D "/usr"
#define EXPECTED_PATH_D "/usr"

char* path_a = NULL;
char* path_b = NULL;
char* path_c = NULL;
char* path_d = NULL;
char* home = NULL;

int
 test_suite_setup (void)  {
  home = path_current_user_home ();

  return 0;
}

int
test_suite_teardown (void) {
  free (path_a);
  free (path_b);
  free (path_c);
  free (path_d);
  free (home);

  return 0;
}

void
test_path_expand_file_tilde (void) {
  path_a = path_expand_tilde (TEST_PATH_A);
  CU_ASSERT (path_a != NULL);
  CU_ASSERT (strcmp (path_a, EXPECTED_PATH_A) == 0);

  path_b = path_expand_tilde (TEST_PATH_B);
  CU_ASSERT (path_b != NULL);
  CU_ASSERT (strcmp (path_b, EXPECTED_PATH_B) == 0);

  path_c = path_expand_tilde (TEST_PATH_C);
  CU_ASSERT (path_c != NULL);
  CU_ASSERT (strcmp (path_c, EXPECTED_PATH_C) == 0);


  path_d = path_expand_tilde (TEST_PATH_D);
  CU_ASSERT (path_d != NULL);
  CU_ASSERT (strcmp (path_d, EXPECTED_PATH_D) == 0);
}


void
test_path_current_user_home (void) {
  const char* home_expected = getenv ("HOME");
  CU_ASSERT (strcmp (home, home_expected) == 0);
}

int main (void) {

  if (CU_initialize_registry () != CUE_SUCCESS) return CU_get_error ();

  CU_pSuite suite = CU_add_suite ("src/util/path-utils", test_suite_setup, test_suite_teardown);

  if (!suite) {
    CU_cleanup_registry ();
    return CU_get_error ();
  }

  if (!CU_add_test (suite, "Test path_exapand_tilde ().",
		    test_path_expand_file_tilde)
      || !CU_add_test (suite, "Test path_current_user_home ().",
		       test_path_current_user_home)) {

    CU_cleanup_registry ();
    return CU_get_error ();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
