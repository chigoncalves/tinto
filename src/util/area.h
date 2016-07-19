/**************************************************************************
* Copyright (C) 2008 thierry lorthiois (lorthiois@bbsoft.fr)
*
* base class for all graphical objects (panel, taskbar, task, systray, clock, ...).
* Area is at the begining of each object (&object == &area).
*
* Area manage the background and border drawing, size and padding.
* Each Area has one Pixmap (pix).
*
* Area manage the tree of all objects. Parent object drawn before child object.
*   panel -> taskbars -> tasks
*         -> systray -> icons
*         -> clock
*
* draw_foreground(obj) and resize(obj) are virtual function.
*
**************************************************************************/

#ifndef AREA_H
#define AREA_H

#include <stdbool.h>

#include <glib.h>
#include <X11/Xlib.h>
#include <cairo.h>
#include <cairo-xlib.h>

#include "misc.h"

typedef struct {
  color_rgba_t color;
  int width;
  int radius;
} border_t;

typedef struct {
  color_rgba_t color;
  border_t border;
} background_t;


// way to calculate the size
// SIZE_BY_LAYOUT objects : taskbar and task
// SIZE_BY_CONTENT objects : clock, battery, launcher, systray
enum { SIZE_BY_LAYOUT, SIZE_BY_CONTENT };

typedef struct {
  rect_t bounds; /*!< Coordinates on screen. */
  Pixmap pixmap;
  background_t* background;
  GSList* children; /* !< A list of children of given Area object. */
  bool visible; /*!< Whether the Area is visible or not. */
	// way to calculate the size (SIZE_BY_CONTENT or SIZE_BY_LAYOUT)
	int size_mode;
	// need to calculate position and width
	int resize;
	// need redraw Pixmap
	int redraw;
	// paddingxlr = horizontal padding left/right
	// paddingx = horizontal padding between childs
	int paddingxlr, paddingx, paddingy;
	// parent Area
	void *parent;
	// panel
	void *panel;

	// each object can overwrite following function
	void (*_draw_foreground)(void *obj, cairo_t *c);
	// update area's content and update size (width/heith).
	// return '1' if size changed, '0' otherwise.
	int (*_resize)(void *obj);
	// after pos/size changed, the rendering engine will call _on_change_layout(Area*)
	int on_changed;
	void (*_on_change_layout)(void *obj);
	const char* (*_get_tooltip_text)(void *obj);
} Area;

// on startup, initialize fixed pos/size
void init_rendering(void *obj, int pos);

void rendering(void *obj);
void size_by_content (Area *a);
void size_by_layout (Area *a, int pos, int level);
// draw background and foreground
void refresh (Area *a);

// generic resize for SIZE_BY_LAYOUT objects
int resize_by_layout(void *obj, int maximum_size);

// set 'redraw' on an area and childs
void set_redraw (Area *a);

// hide/unhide area
void hide(Area *a);
void show(Area *a);

// draw pixmap
void draw (Area *a);
void draw_background (Area *a, cairo_t *c);

void remove_area (Area *a);

void add_area (Area *a);

void
area_destroy (Area *self);

void
area_draw_rect (cairo_t *c, rectf_t rect, double r);

// clear pixmap with transparent color
void
area_clear_pixmap (Pixmap p, rect_t rect);

#endif
