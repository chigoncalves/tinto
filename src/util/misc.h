#ifndef SRC_UTIL_MISC_H
#define SRC_UTIL_MISC_H 1

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

#endif // SRC_UTIL_MISC_H
