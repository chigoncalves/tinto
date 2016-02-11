/*!
 * \file misc.c
 */

#include "conf.h"

#include <errno.h>                 // For `errno' and `EINVAL'.

#include <ctype.h>                 // For `tolower'.
#include <stdbool.h>               // For `bool'.
#include <stdlib.h>                // For `atof' and `free'.
#include <stdint.h>                // For `uint8_t'.
#include <string.h>                // For `strchr', `strdup' and `strlen'.

#include "misc.h"
#include "string-addins.h" // For `strtrim' and `strltrim'.

extern int errno;

static uint8_t
color_rgba_hex_char_to_int (char chr);

/*!
 * \brief Create a new dimension_t from a string.
 *
 * \param str a string containing a size.
 *
 * \retrun a new dimensin_t.
 */
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

/*!
 * \brief Calculate the width according to a size reference.
 *
 * \param dimen a dimension_t
 *
 * \param reference a size to take into account when calculating the size.
 *
 * \return a new size according to \reference.
 */
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

/*!
 * \brief Calculate the height according to a size reference.
 *
 * \param dimen a dimension_t
 *
 * \param reference a size to take into account when calculating the size.
 *
 * \return a new size according to \reference.
 */
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

/*!
 * \brief Compare two rect_t.
 *
 * \return true if their components are equal, false otherwise.
 */
inline bool rect_equals (const rect_t* this, rect_t* that) {
  return this->height == that->height && this->width == that->width
    && this->x == that->x && this->y == that->y;
}

/*!
 * \brief Get the default color, which is a lightgray.
 */
inline color_rgba_t color_rgba_default (void) {
  static const color_rgba_t color = {
    .red = 211,
    .green = 211,
    .blue = 211,
    .alpha = UINT8_MAX,
  };

  return color;
}

/*!
 * \brief Create a new color_rgba_t from a string.
 *
 *  If \str contains a invalid hexadecimal color a default color is returned
 *  and errno is set to EINVAL.
 *
 * \param str a hexadecimal color.
 *
 * \param okay is set to true if an error occurred.
 */
color_rgba_t color_rgba_create (const char* str, bool* okay) {
  if (!str || '#' != *str) {;
    *okay = false;
    return color_rgba_default ();
  }

  errno = 0;
  color_rgba_t color = { .alpha = UINT8_MAX };
  uint8_t first;
  uint8_t second;

  *okay = true;
  switch (strlen (++str)) {
  case 8:
    first = color_rgba_hex_char_to_int (str[6]) * 16;
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    second = color_rgba_hex_char_to_int(str[7]);
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    color.alpha = first + second;
    /* Fall through.*/

  case 6:
    first = color_rgba_hex_char_to_int (str[4]) * 16;
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    second = color_rgba_hex_char_to_int(str[5]);
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    color.blue = first + second;

    first = color_rgba_hex_char_to_int (str[2]) * 16;
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    second = color_rgba_hex_char_to_int(str[3]);
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    color.green = first + second;

    first = color_rgba_hex_char_to_int (str[1]) * 16;
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    second = color_rgba_hex_char_to_int(str[0]);
    if (errno == EINVAL) {
      *okay = false;
      return color_rgba_default ();
    }

    color.red = first + second;
    break;

  default:
    errno = EINVAL;
    *okay = false;
    return color_rgba_default ();
  }

  return color;
}

static uint8_t color_rgba_hex_char_to_int (char chr) {
  chr = tolower (chr);

  errno = 0;
  uint8_t r = 0;
  if (chr >= '0' && chr <= '9')  r = chr - '0';
  else if (chr >= 'a' && chr <= 'f')  r = chr - 'a' + 10;
  else errno = EINVAL;

  return r;
}

/*!
 * \brief Compare two color_rgba_t.
 *
 * \param this a color instance.
 *
 * \param that a color instance.
 *
 * \return true if both colors components are equal, farne othecwise.
 */
inline bool color_rgba_equals (const color_rgba_t* this, const color_rgba_t* that) {
  return this->alpha == that->alpha && this->red == that->red
    && this->green == that->green && this->blue == that->blue;
}

/*!
 * \brief Extract color components to a array.
 *
 * \param self a color_rgba_t instance.
 *
 * \param colors a array.
 */
inline void color_rgba_extract (const color_rgba_t* self, double colors[4]) {
  colors[0] = self->red / 255.0;
  colors[1] = self->green / 255.0;
  colors[2] = self->blue / 255.0;
  colors[3] = self->alpha / 255.0;
}
