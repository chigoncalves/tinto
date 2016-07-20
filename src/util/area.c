
/*! \file area.c */
/**********************************************************************
*
* Tint2 : area
*
* Copyright (C) 2008 thierry lorthiois (lorthiois@bbsoft.fr) from
* Omega distribution
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
**********************************************************************/

#include "conf.h" // For system checks.

#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pango/pangocairo.h>

#include "area.h"
#include "debug.h"
#include "panel.h"
#include "server.h"

extern int errno;

void init_rendering(void *obj, int pos)
{
	Area *a = (Area*)obj;

	// initialize fixed position/size
	GSList *l;
	for (l = a->children; l ; l = l->next) {
		Area *child = ((Area*)l->data);
		if (panel_horizontal) {
		  child->bounds.y = pos + a->background->border.width +
		    a->paddingy;
		  child->bounds.height = a->bounds.height -
		    (2 * (a->background->border.width + a->paddingy));
			if (child->_on_change_layout)
				child->_on_change_layout(child);
			init_rendering(child, child->bounds.y);
		}
		else {
		  child->bounds.x = pos + a->background->border.width
		    + a->paddingy;
		  child->bounds.width = a->bounds.width -
		    (2 * (a->background->border.width + a->paddingy));

			if (child->_on_change_layout)
				child->_on_change_layout(child);
			init_rendering (child, child->bounds.x);
		}
	}
}


void rendering(void *obj)
{
	Panel *panel = (Panel*)obj;

	size_by_content(&panel->area);
	size_by_layout(&panel->area, 0, 1);

	refresh(&panel->area);
}


void size_by_content (Area *a)
{
	// don't resize hiden objects
	if (!a->visible) return;

	// children node are resized before its parent
	GSList *l;
	for (l = a->children; l ; l = l->next)
		size_by_content(l->data);

	// calculate area's size
	a->on_changed = 0;
	if (a->resize && a->size_mode == SIZE_BY_CONTENT) {
		a->resize = 0;

		if (a->_resize) {
			if (a->_resize(a)) {
				// 'size' changed => 'resize = 1' on the parent
				((Area*)a->parent)->resize = 1;
				a->on_changed = 1;
			}
		}
	}
}


void size_by_layout (Area *a, int pos, int level)
{
	// don't resize hiden objects
	if (!a->visible) return;

	// parent node is resized before its children
	// calculate area's size
	GSList *l;
	if (a->resize && a->size_mode == SIZE_BY_LAYOUT) {
		a->resize = 0;

		if (a->_resize) {
			a->_resize(a);
			// resize childs with SIZE_BY_LAYOUT
			for (l = a->children; l ; l = l->next) {
				Area *child = ((Area*)l->data);
				if (child->size_mode == SIZE_BY_LAYOUT && child->children)
					child->resize = 1;
			}
		}
	}

	// update position of childs
	pos += a->paddingxlr + a->background->border.width;
	int i=0;
	for (l = a->children; l ; l = l->next) {
		Area *child = ((Area*)l->data);
		if (!child->visible) continue;
		i++;

		if (panel_horizontal) {
			if (pos != child->bounds.x) {
				// pos changed => redraw
				child->bounds.x = pos;
				child->on_changed = 1;
			}
		}
		else {
			if (pos != child->bounds.y) {
				// pos changed => redraw
				child->bounds.y = pos;
				child->on_changed = 1;
			}
		}

		size_by_layout(child, pos, level+1);

		if (panel_horizontal)
			pos += child->bounds.width + a->paddingx;
		else
			pos += child->bounds.height + a->paddingx;
	}

	if (a->on_changed) {
		// pos/size changed
		a->redraw = 1;
		if (a->_on_change_layout)
			a->_on_change_layout (a);
	}
}


void refresh (Area *a)
{
	// don't draw and resize hide objects
	if (!a->visible) return;

	// don't draw transparent objects (without foreground and without background)
	if (a->redraw) {
		a->redraw = 0;
		// force redraw of child
		//GSList *l;
		//for (l = a->children ; l ; l = l->next)
			//((Area*)l->data)->redraw = 1;

		//printf("draw area posx %d, width %d\n", a->posx, a->width);
		draw(a);
	}

	// draw current Area
	rect_t rect = {
	  .x = 0,
	  .y = 0,
	  .width = a->bounds.width,
	  .height = a->bounds.height
	};
	server_copy_area (&server, a->pixmap,
			  ((Panel*)a->panel)->temp_pmap,
			  rect, (point_t){ a->bounds.x, a->bounds.y});

	// and then refresh child object
	GSList *l;
	for (l = a->children; l ; l = l->next)
		refresh(l->data);
}


int resize_by_layout(void *obj, int maximum_size)
{
	Area *child, *a = (Area*)obj;
	int size, nb_by_content=0, nb_by_layout=0;

	if (panel_horizontal) {
		// detect free size for SIZE_BY_LAYOUT's Area
		size = a->bounds.width - (2 * (a->paddingxlr + a->background->border.width));
		GSList *l;
		for (l = a->children ; l ; l = l->next) {
			child = (Area*)l->data;
			if (child->visible && child->size_mode == SIZE_BY_CONTENT) {
				size -= child->bounds.width;
				nb_by_content++;
			}
			if (child->visible && child->size_mode == SIZE_BY_LAYOUT)
				nb_by_layout++;
		}
		//printf("  resize_by_layout Deb %d, %d\n", nb_by_content, nb_by_layout);
		if (nb_by_content+nb_by_layout)
			size -= ((nb_by_content+nb_by_layout-1) * a->paddingx);

		int width=0, modulo=0, old_width;
		if (nb_by_layout) {
			width = size / nb_by_layout;
			modulo = size % nb_by_layout;
			if (width > maximum_size && maximum_size != 0) {
				width = maximum_size;
				modulo = 0;
			}
		}

		// resize SIZE_BY_LAYOUT objects
		for (l = a->children ; l ; l = l->next) {
			child = (Area*)l->data;
			if (child->visible && child->size_mode == SIZE_BY_LAYOUT) {
				old_width = child->bounds.width;
				child->bounds.width = width;
				if (modulo) {
					child->bounds.width++;
					modulo--;
				}
				if (child->bounds.width != old_width)
					child->on_changed = 1;
			}
		}
	}
	else {
		// detect free size for SIZE_BY_LAYOUT's Area
		size = a->bounds.height - (2 * (a->paddingxlr + a->background->border.width));
		GSList *l;
		for (l = a->children ; l ; l = l->next) {
			child = (Area*)l->data;
			if (child->visible && child->size_mode == SIZE_BY_CONTENT) {
				size -= child->bounds.height;
				nb_by_content++;
			}
			if (child->visible && child->size_mode == SIZE_BY_LAYOUT)
				nb_by_layout++;
		}
		if (nb_by_content+nb_by_layout)
			size -= ((nb_by_content+nb_by_layout-1) * a->paddingx);

		int height=0, modulo=0, old_height;
		if (nb_by_layout) {
			height = size / nb_by_layout;
			modulo = size % nb_by_layout;
			if (height > maximum_size && maximum_size != 0) {
				height = maximum_size;
				modulo = 0;
			}
		}

		// resize SIZE_BY_LAYOUT objects
		for (l = a->children ; l ; l = l->next) {
			child = (Area*)l->data;
			if (child->visible && child->size_mode == SIZE_BY_LAYOUT) {
				old_height = child->bounds.height;
				child->bounds.height = height;
				if (modulo) {
					child->bounds.height++;
					modulo--;
				}
				if (child->bounds.height != old_height)
					child->on_changed = 1;
			}
		}
	}
	return 0;
}


void set_redraw (Area *a)
{
	a->redraw = 1;

	GSList *l;
	for (l = a->children ; l ; l = l->next)
		set_redraw(l->data);
}

void hide(Area *a)
{
	Area *parent = (Area*)a->parent;

	a->visible = 0;
	parent->resize = 1;
	if (panel_horizontal)
		a->bounds.width = 0;
	else
		a->bounds.height = 0;
}

void show(Area *a)
{
	Area *parent = (Area*)a->parent;

	a->visible = 1;
	parent->resize = 1;
	a->resize = 1;
}

void draw (Area *a)
{
	if (a->pixmap) XFreePixmap (server.dsp, a->pixmap);
	a->pixmap = server_create_pixmap (&server,
					  area_get_dimen (a));

	// add layer of root pixmap (or clear pixmap if real_transparency==true)
	if (server.real_transparency) {
	  area_clear_pixmap (a->pixmap,
			     rect_with_size (a->bounds.width,
					     a->bounds.height));
	}

	server_copy_area (&server, ((Panel *)a->panel)->temp_pmap,
			  a->pixmap, a->bounds, (point_t){0, 0});

	cairo_surface_t* cs =
	  server_create_cairo_xlib_surface (&server, a->pixmap,
					    area_get_dimen (a));
	cairo_t* c = cairo_create (cs);

	draw_background (a, c);

	if (a->area_draw_foreground)
		a->area_draw_foreground(a, c);

	cairo_destroy (c);
	cairo_surface_destroy (cs);
}


void draw_background (Area *a, cairo_t *c) {
  if (a->background->color.alpha > 0) {

    progn {
      rectf_t rect = {
	.x = a->background->border.width,
	.y = a->background->border.width,
	.width = a->bounds.width - (2 * a->background->border.width),
	.height = a->bounds.height - (2 * a->background->border.width),

      };
      area_draw_rect (c, rect, a->background->border.radius -
		      a->background->border.width / 1.571);
    }

    double bg_color[4];
    color_rgba_to_array (&a->background->color, bg_color);
    cairo_set_source_rgba (c, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
		cairo_fill(c);
  }

  if (a->background->border.width > 0 && a->background->border.color.alpha > 0) {
		cairo_set_line_width (c, a->background->border.width);

		// draw border inside (x, y, width, height)

    progn {
      rectf_t rect = {
	.x = a->background->border.width / 2.0,
	.y = a->background->border.width / 2.0,
	.width = a->bounds.width - a->background->border.width,
	.height = a->bounds.height - a->background->border.width,
      };
      area_draw_rect (c, rect, a->background->border.radius);

    }
		/*
		// convert : radian = degre * M_PI/180
		// definir le degrade dans un carre de (0,0) (100,100)
		// ensuite ce degrade est extrapoler selon le ratio width/height
		// dans repere (0, 0) (100, 100)
		double X0, Y0, X1, Y1, degre;
		// x = X * (a->width / 100), y = Y * (a->height / 100)
		double x0, y0, x1, y1;
		X0 = 0;
		Y0 = 100;
		X1 = 100;
		Y1 = 0;
		degre = 45;
		// et ensuite faire la changement d'unite du repere
		// car ce qui doit reste inchangee est les traits et pas la direction

		// il faut d'abord appliquer une rotation de 90 (et -180 si l'angle est superieur a  180)
		// ceci peut etre applique une fois pour toute au depart
		// ensuite calculer l'angle dans le nouveau repare
		// puis faire une rotation de 90
		x0 = X0 * ((double)a->width / 100);
		x1 = X1 * ((double)a->width / 100);
		y0 = Y0 * ((double)a->height / 100);
		y1 = Y1 * ((double)a->height / 100);

		x0 = X0 * ((double)a->height / 100);
		x1 = X1 * ((double)a->height / 100);
		y0 = Y0 * ((double)a->width / 100);
		y1 = Y1 * ((double)a->width / 100);

		cairo_pattern_t *linpat;
		linpat = cairo_pattern_create_linear (x0, y0, x1, y1);
		cairo_pattern_add_color_stop_rgba (linpat, 0, a->border.color[0], a->border.color[1], a->border.color[2], a->border.alpha);
		cairo_pattern_add_color_stop_rgba (linpat, 1, a->border.color[0], a->border.color[1], a->border.color[2], 0);
		cairo_set_source (c, linpat);
		*/
    double color[4];
    color_rgba_to_array (&a->background->border.color, color);
    cairo_set_source_rgba (c, color[0], color[1], color[2], color[3]);


		cairo_stroke (c);
		//cairo_pattern_destroy (linpat);
	}
}


void remove_area (Area *a)
{
	Area *parent = (Area*)a->parent;

	parent->children = g_slist_remove(parent->children, a);
	set_redraw (parent);

}


void add_area (Area *a)
{
	Area *parent = (Area*)a->parent;

	parent->children = g_slist_append(parent->children, a);
	set_redraw (parent);

}

void
area_destroy (Area *self) {
  if (!self)
    return;

  GSList* children = self->children;
  while (children) {
    area_destroy (children->data);
    children = children->next;
  }

  if (self->children) {
    g_slist_free (self->children);
    self->children = NULL;
  }
  if (self->pixmap) {
    XFreePixmap (server.dsp, self->pixmap);
    self->pixmap = None;
  }
}

void
area_draw_rect (cairo_t *c, rectf_t rect, double r) {
  if (r > 0.0) {
    double c1 = 0.55228475 * r;

    cairo_move_to (c, rect.x + r, rect.y);
    cairo_rel_line_to(c, rect.width-2*r, 0);
    cairo_rel_curve_to(c, c1, 0.0, r, c1, r, r);
    cairo_rel_line_to(c, 0, rect.height-2*r);
    cairo_rel_curve_to(c, 0.0, c1, c1-r, r, -r, r);
    cairo_rel_line_to (c, -rect.width +2*r, 0);
    cairo_rel_curve_to (c, -c1, 0, -r, -c1, -r, -r);
    cairo_rel_line_to (c, 0, -rect.height + 2 * r);
    cairo_rel_curve_to (c, 0, -c1, r - c1, -r, r, -r);
  }
  else
    cairo_rectangle (c, rect.x, rect.y, rect.width, rect.height);
}

void
area_clear_pixmap (Pixmap p, rect_t rect) {
  Picture pict =
    XRenderCreatePicture (server.dsp, p,
			  XRenderFindVisualFormat (server.dsp,
						   server.visual),
			  0, 0);

  XRenderColor col = { .red = 0, .green = 0, .blue = 0, .alpha = 0 };
  XRenderFillRectangle (server.dsp, PictOpSrc, pict, &col,
			rect.x, rect.y, rect.width, rect.height);
  XRenderFreePicture (server.dsp, pict);
}

inline void
area_set_padding (Area* self, const int vert, const int horiz,
		  int paddx) {
  self->padding.left = self->padding.right = horiz / 2;
  self->padding.bottom = self->padding.top = vert / 2;
  self->paddingxlr = horiz;

  self->paddingy = vert;;
  self->paddingx = paddx;
}

inline void
area_set_margin (Area* self, const int vert, const int horiz) {
  self->margin.bottom = self->margin.top = vert / 2;
  self->margin.left = self->margin.right = horiz / 2;
}

inline dimen_t
area_get_dimen (const Area* self) {
  return (dimen_t){self->bounds.width, self->bounds.height};
}
