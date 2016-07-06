#include "conf.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#include <string.h>

#include <CUnit/CUError.h>
#include <CUnit/Basic.h>

#include "string-addins.h"


#define STR_A_VALUE "            A basic string."
#define STR_A_EXPECTED_VALUE_LT "A basic string."

#define STR_B_VALUE "\nClear as water.\t   \n"
#define STR_B_EXPECTED_VALUE_LT "Clear as water.\t   \n"

#define STR_C_VALUE "\t   \n\t\n    \nCookies  "
#define STR_C_EXPECTED_VALUE_LT "Cookies  "

#define STR_D_VALUE "No need for trimming."
#define STR_D_EXPECTED_VALUE_LT STR_D_VALUE


#define STR_E_VALUE "            A basic string."
#define STR_E_EXPECTED_VALUE_RT "            A basic string."

#define STR_F_VALUE "\nClear as water.\t   \n"
#define STR_F_EXPECTED_VALUE_RT "\nClear as water."

#define STR_G_VALUE "\t   \n\t\n    \nCookies  "
#define STR_G_EXPECTED_VALUE_RT "\t   \n\t\n    \nCookies"

#define STR_H_VALUE "No need for trimming."
#define STR_H_EXPECTED_VALUE_RT STR_H_VALUE


#define STR_I_VALUE "            A basic string."
#define STR_I_EXPECTED_VALUE "A basic string."

#define STR_J_VALUE "Clear as water."
#define STR_J_EXPECTED_VALUE "Clear as water."

#define STR_K_VALUE "\t   \n\t\n    \nCookies  "
#define STR_K_EXPECTED_VALUE "Cookies"

#define STR_L_VALUE "No need for trimming."
#define STR_L_EXPECTED_VALUE STR_D_VALUE


#define WORD_A "Sounds like..."
#define WORD_A_GOOD_SUFFIX  "ike..."
#define WORD_A_BAD_SUFFIX "ike"
#define WORD_A_GOOD_PREFIX "Sounds"
#define WORD_A_BAD_PREFIX "sounds"

#define WORD_B "~/tasks.org"
#define WORD_B_GOOD_SUFFIX ".org"
#define WORD_B_BAD_SUFFIX ".or"
#define WORD_B_BAD_PREFIX "~/foo"
#define WORD_B_GOOD_PREFIX "~/task"
#define WORD_B_BAD_SUFFIX_A "~/tasks.org~"


char* str_a, *str_b, *str_c, *str_d;
char * str_e, *str_f, *str_g, *str_h;
char * str_i, *str_j, *str_k, *str_l;

int
string_addins_setup_suite (void) {
  str_a = strdup (STR_A_VALUE);
  str_b = strdup (STR_B_VALUE);
  str_c = strdup (STR_C_VALUE);
  str_d = strdup (STR_D_VALUE);

  str_e = strdup (STR_E_VALUE);
  str_f = strdup (STR_F_VALUE);
  str_g = strdup (STR_G_VALUE);
  str_h = strdup (STR_H_VALUE);

  str_i = strdup (STR_I_VALUE);
  str_j = strdup (STR_J_VALUE);
  str_k = strdup (STR_K_VALUE);
  str_l = strdup (STR_L_VALUE);

  return 0;
}

int
string_addins_teardown_suite (void) {
  free (str_a);
  free (str_b);
  free (str_c);
  free (str_d);

  free (str_e);
  free (str_f);
  free (str_g);
  free (str_h);

  free (str_i);
  free (str_j);
  free (str_k);
  free (str_l);

  return 0;
}

void test_left_side_trim (void) {
  CU_ASSERT (strcmp (STR_A_EXPECTED_VALUE_LT, strltrim (str_a)) == 0);
  CU_ASSERT (strcmp (STR_B_EXPECTED_VALUE_LT, strltrim (str_b)) == 0);
  CU_ASSERT (strcmp (STR_C_EXPECTED_VALUE_LT, strltrim (str_c)) == 0);
  CU_ASSERT (strcmp (STR_D_EXPECTED_VALUE_LT, strltrim (str_d)) == 0)
}

void test_right_side_trim (void) {
  CU_ASSERT (strcmp (STR_E_EXPECTED_VALUE_RT, strrtrim (str_e)) == 0);
  CU_ASSERT (strcmp (STR_F_EXPECTED_VALUE_RT, strrtrim (str_f)) == 0);
  CU_ASSERT (strcmp (STR_G_EXPECTED_VALUE_RT, strrtrim (str_g)) == 0);
  CU_ASSERT (strcmp (STR_H_EXPECTED_VALUE_RT, strrtrim (str_h)) == 0);
}

void test_both_sides_trim () {
  CU_ASSERT (strcmp (STR_I_EXPECTED_VALUE, strtrim (str_i)) == 0);
  CU_ASSERT (strcmp (STR_J_EXPECTED_VALUE, strtrim (str_j)) == 0);
  CU_ASSERT (strcmp (STR_K_EXPECTED_VALUE, strtrim (str_k)) == 0);
  CU_ASSERT (strcmp (STR_L_EXPECTED_VALUE, strtrim (str_l)) == 0);
}

void test_strendswith (void) {
  CU_ASSERT (strendswith (WORD_A, WORD_A_BAD_SUFFIX) == false);
  CU_ASSERT (strendswith (WORD_A, WORD_A_GOOD_SUFFIX) == true);
  CU_ASSERT (strendswith (WORD_A_GOOD_SUFFIX, WORD_A) == true);
  CU_ASSERT (strendswith (WORD_A_BAD_SUFFIX, WORD_A) == false);
  CU_ASSERT (strendswith (WORD_A, WORD_A) == true);


  CU_ASSERT (strendswith (WORD_B, WORD_B_BAD_SUFFIX) == false);
  CU_ASSERT (strendswith (WORD_B, WORD_B_GOOD_SUFFIX) == true);
  CU_ASSERT (strendswith (WORD_B_GOOD_SUFFIX, WORD_B) == true);
  CU_ASSERT (strendswith (WORD_B_BAD_SUFFIX, WORD_B) == false);
  CU_ASSERT (strendswith (WORD_B, WORD_B) == true);
}

void test_strstartswith (void) {
  CU_ASSERT (strstartswith (WORD_A, WORD_A_GOOD_PREFIX));
  CU_ASSERT (strstartswith( WORD_A, WORD_A_BAD_PREFIX) == false);

  CU_ASSERT (strstartswith (WORD_B, WORD_B_BAD_PREFIX) == false);
  CU_ASSERT (strstartswith (WORD_B, WORD_B_GOOD_PREFIX));
  CU_ASSERT (strstartswith (WORD_B, WORD_B));
  CU_ASSERT (strstartswith (WORD_B, WORD_B_BAD_SUFFIX_A) == false);
}

void
test_string_addins (void) {
  CU_pSuite suite = CU_add_suite ("string-addins",
				  string_addins_setup_suite,
				  string_addins_teardown_suite);
  if (!suite) return;

  if (!CU_add_test(suite, "string-addins/strltrim", test_left_side_trim)
      || !CU_add_test(suite, "string-addins/strtrim", test_both_sides_trim)
      || !CU_add_test(suite, "string-addins/strrtrim", test_right_side_trim)
      || !CU_add_test (suite, "string-addins/strendswith", test_strendswith)
      || !CU_add_test (suite, "string-addins/strstartswith",
		       test_strstartswith))
    return;
}
