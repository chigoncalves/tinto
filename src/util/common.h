#ifndef TINTO_SRC_UTIL_COMMON_H
#define TINTO_SRC_UTIL_COMMON_H


#define WM_CLASS_TINT   "panel"

#include <Imlib2.h>
#include <pango/pangocairo.h>

#include "area.h"
#include "misc.h"

// mouse actions
enum { NONE=0, CLOSE, TOGGLE, ICONIFY, SHADE, TOGGLE_ICONIFY, MAXIMIZE_RESTORE, MAXIMIZE, RESTORE, DESKTOP_LEFT, DESKTOP_RIGHT, NEXT_TASK, PREV_TASK };

#define ALLDESKTOP  0xFFFFFFFF

// extract key = value
int parse_line (char *line, char **key, char **value);

// execute a command by calling fork
void tint_exec(const char* command);

// conversion
int hex_char_to_int (char c);
int hex_to_rgb (char *hex, int *r, int *g, int *b);
void get_color (char *hex, double *rgb);

void extract_values (const char *value, char **value1, char **value2, char **value3);

// adjust Alpha/Saturation/Brightness on an ARGB icon
// alpha from 0 to 100, satur from 0 to 1, bright from 0 to 1.
void adjust_asb(DATA32 *data, int w, int h, int alpha, float satur, float bright);
void createHeuristicMask(DATA32* data, int w, int h);


void
render_image (Drawable d, const rect_t* rect);

void
draw_text (PangoLayout *layout, cairo_t *c, int posx, int posy, const color_rgba_t* color, int font_shadow);

#endif
