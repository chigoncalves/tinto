#ifndef SRC_UTIL_MISC_H
#define SRC_UTIL_MISC_H 1

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  double x;
  double y;
  double width;
  double height;
} rectf_t;

typedef struct {
  int x;
  int y;
  int width;
  int height;
} rect_t;

typedef struct {
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int height;
} urect_t;

typedef enum {
  Pixels,
  Percentage,
} unit_t;

typedef struct {
  double value;
  unit_t unit;
} width_t;

typedef struct {
  double value;
  unit_t unit;
} height_t;

typedef struct {
  width_t width;
  height_t height;
} dimension_t;

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
} color_rgba_t;

dimension_t dimension_create_from_str (char* str);

double
dimension_calculate_width (dimension_t dimen, double reference);

double
dimension_calculate_height (dimension_t dimen, double reference);

bool rect_equals (const rect_t* this, rect_t* that);

color_rgba_t color_rgba_default (void);

color_rgba_t color_rgba_create (const char* str, bool* okay);

bool color_rgba_equals (const color_rgba_t* this, const color_rgba_t* that);

void color_rgba_extract (const color_rgba_t* self, double colors[4]);

#endif // SRC_UTIL_MISC_H
