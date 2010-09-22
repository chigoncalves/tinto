/**************************************************************************
*
* Tint2 : area
*
* Copyright (C) 2008 thierry lorthiois (lorthiois@bbsoft.fr)
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pango/pangocairo.h>

#include "area.h"
#include "server.h"
#include "panel.h"


/************************************************************
 * !!! This design is experimental and not yet fully implemented !!!!!!!!!!!!!
 * 
 * DATA ORGANISATION :
 * Areas in tint2 are similar to widgets in a GUI. 
 * All graphical objects (panel, taskbar, task, systray, clock, ...) 'inherit' an abstract class 'Area'.
 * This class 'Area' manage the background, border, size, position and padding.
 * Area is at the begining of each object (&object == &area).
 * 
 * tint2 define one panel per monitor. And each panel have a tree of Area.
 * The root of the tree is Panel.Area. And task, clock, systray, taskbar,... are nodes.
 * 
 * The tree give the localisation of each object :
 * - tree's root is in the background while tree's leafe are foreground objects
 * - position of a node/Area depend on the layout : parent's position (posx, posy), size of previous brothers and parent's padding
 * - size of a node/Area depend on the content (SIZE_BY_CONTENT objects) or on the layout (SIZE_BY_LAYOUT objects) 
 * 
 * DRAWING AND LAYERING ENGINE :
 * Redrawing an object (like the clock) could come from an 'external event' (date change) 
 * or from a 'layering event' (position change).
 * The following 'drawing engine' take care of :
 * - posx/posy of all Area
 * - 'layering event' propagation between object
 * 1) browse tree SIZE_BY_CONTENT
 *  - resize SIZE_BY_CONTENT node : children are resized before parent
 * 	- if 'size' changed then 'resize = 1' on the parent
 * 2) browse tree SIZE_BY_LAYOUT and POSITION
 *  - resize SIZE_BY_LAYOUT node : parent is resized before children
 * 	- if 'size' changed then 'resize = 1' on childs with SIZE_BY_LAYOUT
 *  - calculate position (posx,posy) : parent is calculated before children
 * 	- if 'position' changed then 'redraw = 1'
 * 3) browse tree REDRAW
 *  - redraw needed objects : parent is drawn before children
 *
 * CONFIGURE PANEL'S LAYOUT :
 * 'panel_items' parameter (in config) define the list and the order of nodes in tree's panel.
 * 'panel_items = SC' define a panel with just Systray and Clock.
 * So the tree 'Panel.Area' will have 2 childs (Systray and Clock).
 *
 ************************************************************/

void init_rendering(void *obj, int pos)
{
	Area *a = (Area*)obj;
	
	// initialize fixed position/size
	GSList *l;
	for (l = a->list; l ; l = l->next) {
		Area *child = ((Area*)l->data);
		if (panel_horizontal) {
			child->posy = pos + a->bg->border.width + a->paddingy;
			child->height = a->height - (2 * (a->bg->border.width + a->paddingy));
			init_rendering(child, child->posy);
		}
		else {
			child->posx = pos + a->bg->border.width + a->paddingy;
			child->width = a->width - (2 * (a->bg->border.width + a->paddingy));
			init_rendering(child, child->posx);
		}
	}
}


void rendering(void *obj)
{
	Panel *panel = (Panel*)obj;
	
	size_by_content(&panel->area);
	size_by_layout(&panel->area, 0, 0);
	
	refresh(&panel->area);
}


void size_by_content (Area *a)
{
	// don't resize hiden objects
	if (!a->on_screen) return;

	// children node are resized before its parent
	GSList *l;
	for (l = a->list; l ; l = l->next)
		size_by_content(l->data);
	
	// calculate area's size
	if (a->resize && a->size_mode == SIZE_BY_CONTENT) {
		a->resize = 0;

		if (a->_resize) {
			if (a->_resize(a)) {
				// 'size' changed => 'resize = 1' on the parent and redraw object
				((Area*)a->parent)->resize = 1;
			}
			a->redraw = 1;
		}
	}
}


void size_by_layout (Area *a, int pos, int level)
{
	// don't resize hiden objects
	if (!a->on_screen) return;

	// parent node is resized before its children
	// calculate area's size
	GSList *l;
	if (a->resize && a->size_mode == SIZE_BY_LAYOUT) {
		a->resize = 0;

		if (a->_resize) {
			if (a->_resize(a)) {
				// if 'size' changed then 'resize = 1' on childs with SIZE_BY_LAYOUT
				for (l = a->list; l ; l = l->next) {
					if (((Area*)l->data)->size_mode == SIZE_BY_LAYOUT)
						((Area*)l->data)->resize = 1;
				}
			}
		}
	}

	// update position of childs
	pos += a->paddingxlr + a->bg->border.width;
	int i=0;
	for (l = a->list; l ; l = l->next) {
		Area *child = ((Area*)l->data);
		if (!child->on_screen) continue;
		i++;
		
		if (panel_horizontal) {
			if (pos != child->posx) {
				// pos changed => redraw
				child->posx = pos;
				child->redraw = 1;
			}
		}
		else {
			if (pos != child->posy) {
				// pos changed => redraw
				child->posy = pos;
				child->redraw = 1;
			}
		}
		//printf("level %d, object %d, pos %d\n", level, i, pos);
		
		size_by_layout(child, pos, level+1);
		
		if (panel_horizontal)
			pos += child->width + a->paddingx;
		else
			pos += child->height + a->paddingx;
	}
}


void refresh (Area *a)
{
	// don't draw and resize hide objects
	if (!a->on_screen) return;

	// don't draw transparent objects (without foreground and without background)
	if (a->redraw) {
		a->redraw = 0;
		// force redraw of child
		GSList *l;
		for (l = a->list ; l ; l = l->next)
			((Area*)l->data)->redraw = 1;
//			set_redraw(l->data);

		//printf("draw area posx %d, width %d\n", a->posx, a->width);
		draw(a);
	}

	// draw current Area
	if (a->pix == 0) printf("empty area posx %d, width %d\n", a->posx, a->width);
	XCopyArea (server.dsp, a->pix, ((Panel *)a->panel)->temp_pmap, server.gc, 0, 0, a->width, a->height, a->posx, a->posy);

	// and then refresh child object
	GSList *l;
	for (l = a->list; l ; l = l->next)
		refresh(l->data);
}


int resize_by_layout(void *obj)
{
	Taskbar *taskbar = (Taskbar*)obj;
	Panel *panel = (Panel*)taskbar->area.panel;
	Task *tsk;
	GSList *l;
	int  task_count, border_width;

	//printf("resize_taskbar : posx et width des taches\n");
	taskbar->area.redraw = 1;

	border_width = taskbar->area.bg->border.width;

	if (panel_horizontal) {
		int  pixel_width, modulo_width=0;
		int  taskbar_width;

		// new task width for 'desktop'
		task_count = g_slist_length(taskbar->area.list);
		if (!task_count) pixel_width = panel->g_task.maximum_width;
		else {
			taskbar_width = taskbar->area.width - (2 * border_width) - (2 * panel->g_taskbar.area.paddingxlr);
			if (task_count>1) taskbar_width -= ((task_count-1) * panel->g_taskbar.area.paddingx);

			pixel_width = taskbar_width / task_count;
			if (pixel_width > panel->g_task.maximum_width)
				pixel_width = panel->g_task.maximum_width;
			else
				modulo_width = taskbar_width % task_count;
		}

		taskbar->task_width = pixel_width;
		taskbar->task_modulo = modulo_width;
		taskbar->text_width = pixel_width - panel->g_task.text_posx - panel->g_task.area.bg->border.width - panel->g_task.area.paddingx;

		// change pos_x and width for all tasks
		for (l = taskbar->area.list; l ; l = l->next) {
			tsk = l->data;
			if (!tsk->area.on_screen) continue;
			set_task_redraw(tsk);  // always redraw task, because the background could have changed (taskbar_active_id)
			tsk->area.width = pixel_width;
// TODO : move later (when posx is known)
//			long value[] = { panel->posx+x, panel->posy, pixel_width, panel->area.height };
//			XChangeProperty (server.dsp, tsk->win, server.atom._NET_WM_ICON_GEOMETRY, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)value, 4);

			if (modulo_width) {
				tsk->area.width++;
				modulo_width--;
			}
		}
	}
	else {
		int  pixel_height, modulo_height=0;
		int  taskbar_height;

		// new task width for 'desktop'
		task_count = g_slist_length(taskbar->area.list);
		if (!task_count) pixel_height = panel->g_task.maximum_height;
		else {
			taskbar_height = taskbar->area.height - (2 * border_width) - (2 * panel->g_taskbar.area.paddingxlr);
			if (task_count>1) taskbar_height -= ((task_count-1) * panel->g_taskbar.area.paddingx);

			pixel_height = taskbar_height / task_count;
			if (pixel_height > panel->g_task.maximum_height)
				pixel_height = panel->g_task.maximum_height;
			else
				modulo_height = taskbar_height % task_count;
		}

		taskbar->task_width = pixel_height;
		taskbar->task_modulo = modulo_height;
		taskbar->text_width = taskbar->area.width - (2 * panel->g_taskbar.area.paddingy) - panel->g_task.text_posx - panel->g_task.area.bg->border.width - panel->g_task.area.paddingx;

		// change pos_y and height for all tasks
		for (l = taskbar->area.list; l ; l = l->next) {
			tsk = l->data;
			if (!tsk->area.on_screen) continue;
			set_task_redraw(tsk);  // always redraw task, because the background could have changed (taskbar_active_id)
			tsk->area.height = pixel_height;
// TODO : move later (when posy is known)
//			long value[] = { panel->posx, panel->posy+y, panel->area.width, pixel_height };
//			XChangeProperty (server.dsp, tsk->win, server.atom._NET_WM_ICON_GEOMETRY, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)value, 4);

			if (modulo_height) {
				tsk->area.height++;
				modulo_height--;
			}
		}
	}
	return 0;
}


void set_redraw (Area *a)
{
	a->redraw = 1;

	GSList *l;
	for (l = a->list ; l ; l = l->next)
		set_redraw(l->data);
}

void hide(Area *a)
{
	Area *parent = (Area*)a->parent;

	a->on_screen = 0;
	parent->resize = 1;
	if (panel_horizontal)
		a->width = 0;
	else
		a->height = 0;
}

void show(Area *a)
{
	Area *parent = (Area*)a->parent;

	a->on_screen = 1;
	parent->resize = 1;
	a->resize = 1;
}

void draw (Area *a)
{
	if (a->pix) XFreePixmap (server.dsp, a->pix);
	a->pix = XCreatePixmap (server.dsp, server.root_win, a->width, a->height, server.depth);

	// add layer of root pixmap (or clear pixmap if real_transparency==true)
	if (server.real_transparency)
		clear_pixmap(a->pix, 0 ,0, a->width, a->height);
	XCopyArea (server.dsp, ((Panel *)a->panel)->temp_pmap, a->pix, server.gc, a->posx, a->posy, a->width, a->height, 0, 0);

	cairo_surface_t *cs;
	cairo_t *c;

	cs = cairo_xlib_surface_create (server.dsp, a->pix, server.visual, a->width, a->height);
	c = cairo_create (cs);

	draw_background (a, c);

	if (a->_draw_foreground)
		a->_draw_foreground(a, c);

	cairo_destroy (c);
	cairo_surface_destroy (cs);
}


void draw_background (Area *a, cairo_t *c)
{
	if (a->bg->back.alpha > 0.0) {
		//printf("    draw_background (%d %d) RGBA (%lf, %lf, %lf, %lf)\n", a->posx, a->posy, pix->back.color[0], pix->back.color[1], pix->back.color[2], pix->back.alpha);
		draw_rect(c, a->bg->border.width, a->bg->border.width, a->width-(2.0 * a->bg->border.width), a->height-(2.0*a->bg->border.width), a->bg->border.rounded - a->bg->border.width/1.571);
		cairo_set_source_rgba(c, a->bg->back.color[0], a->bg->back.color[1], a->bg->back.color[2], a->bg->back.alpha);
		cairo_fill(c);
	}

	if (a->bg->border.width > 0 && a->bg->border.alpha > 0.0) {
		cairo_set_line_width (c, a->bg->border.width);

		// draw border inside (x, y, width, height)
		draw_rect(c, a->bg->border.width/2.0, a->bg->border.width/2.0, a->width - a->bg->border.width, a->height - a->bg->border.width, a->bg->border.rounded);
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
		cairo_set_source_rgba (c, a->bg->border.color[0], a->bg->border.color[1], a->bg->border.color[2], a->bg->border.alpha);

		cairo_stroke (c);
		//cairo_pattern_destroy (linpat);
	}
}


void remove_area (Area *a)
{
	Area *parent = (Area*)a->parent;

	parent->list = g_slist_remove(parent->list, a);
	set_redraw (parent);

}


void add_area (Area *a)
{
	Area *parent = (Area*)a->parent;

	parent->list = g_slist_remove(parent->list, a);
	set_redraw (parent);

}


void free_area (Area *a)
{
	GSList *l0;
	for (l0 = a->list; l0 ; l0 = l0->next)
		free_area (l0->data);

	if (a->list) {
		g_slist_free(a->list);
		a->list = 0;
	}
	if (a->pix) {
		XFreePixmap (server.dsp, a->pix);
		a->pix = 0;
	}
}


void draw_rect(cairo_t *c, double x, double y, double w, double h, double r)
{
	if (r > 0.0) {
		double c1 = 0.55228475 * r;

		cairo_move_to(c, x+r, y);
		cairo_rel_line_to(c, w-2*r, 0);
		cairo_rel_curve_to(c, c1, 0.0, r, c1, r, r);
		cairo_rel_line_to(c, 0, h-2*r);
		cairo_rel_curve_to(c, 0.0, c1, c1-r, r, -r, r);
		cairo_rel_line_to (c, -w +2*r, 0);
		cairo_rel_curve_to (c, -c1, 0, -r, -c1, -r, -r);
		cairo_rel_line_to (c, 0, -h + 2 * r);
		cairo_rel_curve_to (c, 0, -c1, r - c1, -r, r, -r);
	}
	else
		cairo_rectangle(c, x, y, w, h);
}


void clear_pixmap(Pixmap p, int x, int y, int w, int h)
{
	Picture pict = XRenderCreatePicture(server.dsp, p, XRenderFindVisualFormat(server.dsp, server.visual), 0, 0);
	XRenderColor col = { .red=0, .green=0, .blue=0, .alpha=0 };
	XRenderFillRectangle(server.dsp, PictOpSrc, pict, &col, x, y, w, h);
	XRenderFreePicture(server.dsp, pict);
}
