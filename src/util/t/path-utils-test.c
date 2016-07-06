#include "conf.h"

#include <stdlib.h> // For `free', `getenv'.
#include <string.h> // For `strdup'.

#include <CUnit/Basic.h>


#include "debug.h"
#include "path-utils.h"

#ifdef CONCAT_USER_HOME
#undef CONCAT_USER_HOME
#endif

#ifdef CONCAT
#undef CONCAT
#endif

#define CONCAT(A, B) A # B

#define CONCAT_USER_HOME(path) CONCAT (USER_HOME_DIR, path)

void
test_path_expand_file_tilde (void) {
  char* val = path_expand_tilde ("~");
  CU_ASSERT_PTR_NOT_NULL (val);
  CU_ASSERT_STRING_EQUAL (USER_HOME_DIR, val);
  free (val);

  val = path_expand_tilde ("~/");
  CU_ASSERT_STRING_EQUAL (CONCAT_USER_HOME (/), val);
  free (val);

  val = path_expand_tilde ("~/acme-format.lsp");
  CU_ASSERT_STRING_EQUAL (CONCAT_USER_HOME (/acme-format.lsp), val);
  free (val);

  val = path_expand_tilde ("~acme.asd");
  CU_ASSERT_STRING_NOT_EQUAL (CONCAT_USER_HOME (/acme.asd), val);
  free (val);

  val = path_expand_tilde ("/foo");
  CU_ASSERT_STRING_EQUAL ("/foo", val);
  free (val);

  val = path_expand_tilde ("foo");
  CU_ASSERT_STRING_EQUAL ("foo", val);
  free (val);
}

void
test_path_current_user_home (void) {
  const char* home_expected = getenv ("HOME");
  char* home = path_current_user_home ();
  CU_ASSERT_PTR_NOT_NULL_FATAL (home);
  CU_ASSERT_STRING_EQUAL (home_expected, home);
  free (home);
}

#include <stdio.h>

void
test_path_unexpand_tilde (void) {
  char* val = path_unexpand_tilde (USER_HOME_DIR);
  CU_ASSERT_PTR_NOT_NULL (val);
  CU_ASSERT_STRING_EQUAL ("~", val);
  free (val);

  val = path_unexpand_tilde (CONCAT_USER_HOME (/));
  CU_ASSERT_PTR_NOT_NULL (val);
  CU_ASSERT_STRING_EQUAL ("~/", val);
  free (val);


  val = path_unexpand_tilde (CONCAT_USER_HOME (/acme.asd));
  CU_ASSERT_PTR_NOT_NULL (val);
  CU_ASSERT_STRING_EQUAL ("~/acme.asd", val);
  free (val);

  val = path_unexpand_tilde ("~acme.asd");
  CU_ASSERT_PTR_NOT_NULL (val);
  CU_ASSERT_STRING_NOT_EQUAL ("~/acme.asd", val);
  free (val);

  val = path_unexpand_tilde ("/usr/local");
  CU_ASSERT_PTR_NOT_NULL (val);
  CU_ASSERT_STRING_EQUAL ("/usr/local", val);
  free (val);
}

void
test_path_utils (void) {
  CU_pSuite suite = CU_add_suite ("path-utils",
				  NULL,
				  NULL);

  if (!suite) return;

  if (!CU_add_test (suite, "path-utils/path-exapand-tilde",
  		    test_path_expand_file_tilde)
      || !CU_add_test (suite, "path-utils/path-current-user-home.",
  		       test_path_current_user_home)
      || !CU_add_test (suite, "path-utils/path-unexpand-tilde",
  		       test_path_unexpand_tilde))
    return;
}
