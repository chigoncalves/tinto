/*!
 * \file misc.h
 */

#ifndef TINTO_SRC_UTIL_MISC_H
#define TINTO_SRC_UTIL_MISC_H 1

#include <stdbool.h>
#include <stdint.h>

/*!
 * \struct point_t.
 *
 * \brief A point representation.
 */
typedef struct {
  int x;                /*!<X component. */
  int y;                /*!<Y component. */
} point_t;

/*!
 * \struct rectf_t.
 *
 * \brief a rectangle for float.
 */
typedef struct {
  double x;       /*!<rectangle's x position */
  double y;	  /*!<rectangle's y position  */
  double width;	  /*!<rectangle's width  */
  double height;  /*!<rectangle's height. */
} rectf_t;

/*!
 * \struct rect_t
 *
 * \brief a rectangle for integers.
 */
typedef struct {
  int x;         /*!<rectangle's x position */
  int y;         /*!<rectangle's y position  */
  int width;     /*!<rectangle's width  */
  int height;    /*!<rectangle's height. */
} rect_t;

/*!
 * \enum unit_t.
 *
 * \brief size units.
 */
typedef enum {
  Pixels,        /*!< pixels. */
  Percentage,    /*!< percentage. */
} unit_t;

typedef enum {
  Italic,
  Normal,
} font_style_t;

typedef enum {
  Bold,
  Nil,
} font_weight_t;

/*! \enum value_t.
 *
 * \brief A vlue type.
 */
typedef struct {
  double value;                /*!< The value. */
  unit_t unit;                 /*!< Unit, such as pixels or percentage. */
} value_t;

/*!
 * \struct dimension_t.
 *
 * \brief represents a dimension.
 */
typedef struct {
  value_t width;
  value_t height;
} dimension_t;

typedef struct  {
  int width;
  int height;
} dimen_t;

/*!
 * \struct color_rgba_t.
 *
 * \brief Represens a RGBA color.
 */
typedef struct {
  uint8_t red;                 /*!< Colors Red component. */
  uint8_t green;               /*!< Colors Green component. */
  uint8_t blue;		       /*!< Colors Blue component. */
  uint8_t alpha;	       /*!< Colors Alpha component. */
} color_rgba_t;

typedef struct {
  double red;
  double green;
  double blue;
  double alpha;
} colorf_rgba_t;

typedef struct {
  const char* name;
  double size;
  font_style_t style;
  font_weight_t weight;
  color_rgba_t color;
} font_t;

typedef struct {
  font_t font;
  point_t position;
  char* str;
  bool shadow;
} text_t;

/*! Create a new dimension_t from a string. */
dimension_t
dimension_create_from_str (char* str);

/*! Calculate the width according to a size reference. */
double
dimension_calculate_width (dimension_t dimen, double reference);

/*! Calculate the height according to a size reference. */
double
dimension_calculate_height (dimension_t dimen, double reference);

/*! Compare two rect_t. */
bool
rect_equals (const rect_t* this, rect_t* that);

/*! Get the default color. */
color_rgba_t
color_rgba_default (void);

/*! Create a new color_rgba_t from a string. */
color_rgba_t
color_rgba_create (const char* str, bool* okay);

/*! Compare two color_rgba_t. */
bool
color_rgba_equals (const color_rgba_t* this, const color_rgba_t* that);

/*! Covert a hex character to a integer. */
void
color_rgba_to_array (const color_rgba_t* color, double colors [static 4]);

colorf_rgba_t
color_rgba_to_f (const color_rgba_t* self);

rect_t
rect_with_size (const int width, const int height);

#endif // TINTO_SRC_UTIL_MISC_H
