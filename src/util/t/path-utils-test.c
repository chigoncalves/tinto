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

#define TEST_PATH_E "/home/crowseye"
#define TEST_PATH_E_EXPECTED "~"
#define TEST_PATH_F "/home/crowseye/"
#define TEST_PATH_F_EXPECTED "~/"
#define TEST_PATH_G "/home/crowseye/foo.pl"
#define TEST_PATH_G_EXPECTED "~/foo.pl"
#define TEST_PATH_H "/usr"
#define TEST_PATH_H_EXPECTED "/usr"

char* path_a = NULL;
char* path_b = NULL;
char* path_c = NULL;
char* path_d = NULL;
char* unexpand_path_e = NULL;
char* unexpand_path_f = NULL;
char* unexpand_path_g = NULL;
char* unexpand_path_h = NULL;
char* home = NULL;

int
path_utils_setup_suite (void)  {
  home = path_current_user_home ();

  unexpand_path_e = path_unexpand_tilde (TEST_PATH_E);
  unexpand_path_f = path_unexpand_tilde (TEST_PATH_F);
  unexpand_path_g = path_unexpand_tilde (TEST_PATH_G);
  unexpand_path_h = path_unexpand_tilde (TEST_PATH_H);

  return 0;
}

int
path_utils_teardown_suite (void) {
  free (path_a);
  free (path_b);
  free (path_c);
  free (path_d);
  free (home);

  free (unexpand_path_e);
  free (unexpand_path_f);
  free (unexpand_path_g);
  free (unexpand_path_h);

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

#include <stdio.h>

void
test_path_unexpand_tilde (void) {
  CU_ASSERT (unexpand_path_e != NULL);
  CU_ASSERT (strcmp (TEST_PATH_E_EXPECTED, unexpand_path_e) == 0);

  CU_ASSERT (unexpand_path_f != NULL);
  CU_ASSERT (strcmp (TEST_PATH_F_EXPECTED, unexpand_path_f) == 0);

  CU_ASSERT (unexpand_path_g != NULL);
  CU_ASSERT (strcmp (TEST_PATH_G_EXPECTED, unexpand_path_g) == 0);

  CU_ASSERT (unexpand_path_h != NULL);
  CU_ASSERT (strcmp (TEST_PATH_H_EXPECTED, unexpand_path_h) == 0);
}

void
test_path_utils (void) {
  CU_pSuite suite = CU_add_suite ("path-utils",
				  path_utils_setup_suite,
				  path_utils_teardown_suite);

  if (!suite) return;

  if (!CU_add_test (suite, "path-utils/path-exapand-tilde",
		    test_path_expand_file_tilde)
      || !CU_add_test (suite, "path-utils/path-current-user-home.",
		       test_path_current_user_home)
      || !CU_add_test (suite, "path-utils/path-unexpand-tilde",
		       test_path_unexpand_tilde))
    return;
}
