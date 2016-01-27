#include "conf.h"

#include <stdlib.h> // For `atof' and `free'.
#include <string.h> // For `strchr' and `strlen'.

#include "misc.h"
#include "string-addins.h" // For `strtrim' and `strltrim'.

dimension_t
dimension_create_from_str (char* str) {
  dimension_t dimen = {
    .width = {
      .value = 0.0,
      .unit = Pixels,
    },

    .height = {
      .value = 0.0,
      .unit = Pixels,
    },
  };

  if (!str) return dimen;

  str = strtrim (str);
  char* ptr = strchr (str, ' ');

  if (!ptr) return dimen;

  *ptr++ = NUL;
  ptr = strdup (strltrim (ptr));

  if (strendswith (str, "%")) {
    str[strlen (str) - 1] = NUL;
    dimen.width.value =  atof (str);
    dimen.width.unit = Percentage;
  }
  else if (strendswith (str, "px")) {
    str[strlen (str) - 2] = NUL;
    dimen.width.value =  atof (str);
    dimen.width.unit = Pixels;
  }

  if (strendswith (ptr, "%")) {
    ptr[strlen (ptr) - 1] = NUL;
    dimen.height.value =  atof (ptr);
    dimen.height.unit = Percentage;
  }
  else if (strendswith (ptr, "px")) {
    ptr[strlen (ptr) - 2] = NUL;
    dimen.height.value =  atof (ptr);
    dimen.height.unit = Pixels;
  }

  free (ptr);
  return dimen;
}


double
dimension_calculate_width (dimension_t dimen, double reference) {
  switch (dimen.width.unit) {
  case Pixels:
    return dimen.width.value;

  case Percentage:
    return reference * dimen.width.value / 100.0;

  default:
    return 0.0;
  }
}

double
dimension_calculate_height (dimension_t dimen, double reference) {
  switch (dimen.height.unit) {
  case Pixels:
    return dimen.width.value;

  case Percentage:
    return reference * dimen.height.value / 100.0;

  default:
    return 0.0;
  }
}
