#ifndef SRC_UTIL_MISC_H
#define SRC_UTIL_MISC_H 1

#include <stdbool.h>


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


dimension_t dimension_create_from_str (char* str);

double
dimension_calculate_width (dimension_t dimen, double reference);

double
dimension_calculate_height (dimension_t dimen, double reference);

bool rect_equals (const rect_t* this, rect_t* that);

#endif // SRC_UTIL_MISC_H
