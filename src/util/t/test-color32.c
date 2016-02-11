#include "conf.h"

#include <errno.h>
#include <stdbool.h>

#include <CUnit/Basic.h>

#include "misc.h"

extern int errno;

struct params {
  const char* hex;
  color_rgba_t actual;
  color_rgba_t expected;
} red, green, blue, cyan, orange, black, white, yellow, pink;

int test_suite_setup (void) {
  red.hex = "#ff0000";
  red.expected.red = 255;
  red.expected.blue = red.expected.green = 0;
  red.expected.alpha = UINT8_MAX;

  green.hex = "#00ff00";
  green.expected.green = 255;
  green.expected.red = green.expected.blue = 0;
  green.expected.alpha = UINT8_MAX;

  blue.hex = "#0000ff";
  blue.expected.blue = 255;
  blue.expected.red = blue.expected.green = 0;
  blue.expected.alpha = UINT8_MAX;

  cyan.hex = "#00ffff";
  cyan.expected.red = 0;
  cyan.expected.green = 255;
  cyan.expected.blue = 255;
  cyan.expected.alpha = UINT8_MAX;

  orange.hex = "#ffa500";
  orange.expected.red = 255;
  orange.expected.green = 165;
  orange.expected.blue = 0;
  orange.expected.alpha = UINT8_MAX;

  yellow.hex = "#ffff00";
  yellow.expected.red = 255;
  yellow.expected.green = 255;
  yellow.expected.blue = 0;
  yellow.expected.alpha = UINT8_MAX;

  black.hex = "#000000";
  black.expected.red = 0;
  black.expected.green = 0;
  black.expected.blue = 0;
  black.expected.alpha = UINT8_MAX;

  pink.hex = "#ffc0cb";
  pink.expected.red = 255;
  pink.expected.green = 192;
  pink.expected.blue = 203;
  pink.expected.alpha = UINT8_MAX;

  white.hex = "#ffffff";
  white.expected.red = 255;
  white.expected.green = 255;
  white.expected.blue = 255;
  white.expected.alpha = UINT8_MAX;

  return 0;
}

void test_color_rgba_create (void) {
  bool okay;

  red.actual = color_rgba_create (red.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&red.actual, &red.expected));
  CU_ASSERT_TRUE(okay);

  green.actual = color_rgba_create (green.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&green.actual, &green.expected));
  CU_ASSERT_TRUE(okay);

  blue.actual = color_rgba_create (blue.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&blue.actual, &blue.expected));
  CU_ASSERT_TRUE(okay);

  cyan.actual = color_rgba_create (cyan.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&cyan.actual, &cyan.expected));
  CU_ASSERT_TRUE(okay);

  orange.actual = color_rgba_create (orange.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&orange.actual, &orange.expected));
  CU_ASSERT_TRUE(okay);

  white.actual = color_rgba_create (white.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&white.actual, &white.expected));
  CU_ASSERT_TRUE(okay);

  white.actual = color_rgba_create (white.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&white.actual, &white.expected));
  CU_ASSERT_TRUE(okay);

  yellow.actual = color_rgba_create (yellow.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&yellow.actual, &yellow.expected));
  CU_ASSERT_TRUE(okay);

  black.actual = color_rgba_create (black.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&black.actual, &black.expected));
  CU_ASSERT_TRUE(okay);

  pink.actual = color_rgba_create (pink.hex, &okay);
  CU_ASSERT_TRUE(color_rgba_equals (&pink.actual, &pink.expected));
  CU_ASSERT_TRUE(okay);
}

int main (void) {

  if (CU_initialize_registry () != CUE_SUCCESS) return CU_get_error ();

  CU_pSuite suite = CU_add_suite ("src/util/color_rgba", test_suite_setup,
				  NULL);

  if (!suite) {
    CU_cleanup_registry ();
    return CU_get_error ();
  }

  if (!CU_add_test (suite, "Test color_rgba_create ().",
		    test_color_rgba_create)) {

    CU_cleanup_registry ();
    return CU_get_error ();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
