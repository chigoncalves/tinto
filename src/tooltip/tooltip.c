/**************************************************************************
*
* Copyright (C) 2009 Andreas.Fink (Andreas.Fink85@gmail.com)
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

#include "conf.h" // For `UNUSED'.

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>
#include <cairo-xlib.h>

#include "server.h"
#include "tooltip.h"
#include "panel.h"
#include "timer.h"

static int x, y, width, height;
static int just_shown;

// the next functions are helper functions for tooltip handling
void start_show_timeout();
void start_hide_timeout();
void stop_tooltip_timeout();

Tooltip g_tooltip;


void default_tooltip (void) {
	// give the tooltip some reasonable default values
	memset(&g_tooltip, 0, sizeof(Tooltip));

  bool okay;
  g_tooltip.font_color = color_rgba_create ("#000000", &okay);
  UNUSED (okay);
	just_shown = 0;
}

void cleanup_tooltip()
{
	stop_tooltip_timeout();
	tooltip_hide(NULL);
	tooltip_copy_text(NULL);
	if (g_tooltip.window)
		XDestroyWindow(server.dsp, g_tooltip.window);
	g_tooltip.window = 0;
	pango_font_description_free(g_tooltip.font_desc);
	g_tooltip.font_desc = NULL;
}


void init_tooltip()
{
	if (!g_tooltip.font_desc)
		g_tooltip.font_desc = pango_font_description_from_string(DEFAULT_FONT);
	if (g_tooltip.background == 0)
		g_tooltip.background = &g_array_index(backgrounds, background_t, 0);

	XSetWindowAttributes attr;
	attr.override_redirect = True;
	attr.event_mask = StructureNotifyMask;
	attr.colormap = server.colormap;
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	unsigned long mask = CWEventMask|CWColormap|CWBorderPixel|CWBackPixel|CWOverrideRedirect;
	if (g_tooltip.window)
		XDestroyWindow(server.dsp, g_tooltip.window);
	g_tooltip.window = XCreateWindow(server.dsp, server.root_win, 0, 0, 100, 20, 0, server.depth, InputOutput, server.visual, mask, &attr);
}


void tooltip_trigger_show(Area* area, Panel* p, XEvent *e)
{
	// Position the tooltip in the center of the area
	x = area->bounds.x + MIN(area->bounds.width / 3, 22) + e->xmotion.x_root - e->xmotion.x;
	y = area->bounds.y + area->bounds.height / 2 + e->xmotion.y_root - e->xmotion.y;
	just_shown = 1;
	g_tooltip.panel = p;
	if (g_tooltip.mapped && g_tooltip.area != area) {
		tooltip_copy_text(area);
		tooltip_update();
		stop_tooltip_timeout();
	}
	else if (!g_tooltip.mapped) {
		start_show_timeout();
	}
}


void tooltip_show (void* arg) {
  UNUSED (arg);
  point_t point;
	Window w;
	XTranslateCoordinates ( server.dsp, server.root_win, g_tooltip.panel->main_win, x, y, &point.x, &point.y, &w);
	Area* area;
	area = panel_click_area (g_tooltip.panel, point);
	if (!g_tooltip.mapped && area->_get_tooltip_text) {
		tooltip_copy_text(area);
		g_tooltip.mapped = True;
		XMapWindow(server.dsp, g_tooltip.window);
		tooltip_update();
		XFlush(server.dsp);
	}
}


void tooltip_update_geometry()
{
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout* layout;
	cs = cairo_xlib_surface_create(server.dsp, g_tooltip.window, server.visual, width, height);
	c = cairo_create(cs);
	layout = pango_cairo_create_layout(c);
	pango_layout_set_font_description(layout, g_tooltip.font_desc);
	pango_layout_set_text(layout, g_tooltip.tooltip_text, -1);
	PangoRectangle r1, r2;
	pango_layout_get_pixel_extents(layout, &r1, &r2);
	width = 2*g_tooltip.background->border.width + 2*g_tooltip.paddingx + r2.width;
	height = 2*g_tooltip.background->border.width + 2*g_tooltip.paddingy + r2.height;

	Panel* panel = g_tooltip.panel;
	if (panel_horizontal && panel_position & BOTTOM)
		y = panel->posy-height;
	else if (panel_horizontal && panel_position & TOP)
		y = panel->posy + panel->area.bounds.height;
	else if (panel_position & LEFT)
		x = panel->posx + panel->area.bounds.width;
	else
		x = panel->posx - width;

	g_object_unref(layout);
	cairo_destroy(c);
	cairo_surface_destroy(cs);
}


void tooltip_adjust_geometry()
{
	// adjust coordinates and size to not go offscreen
	// it seems quite impossible that the height needs to be adjusted, but we do it anyway.

	int min_x, min_y, max_width, max_height;
	Panel* panel = g_tooltip.panel;
	int screen_width = server.monitor[panel->monitor].x + server.monitor[panel->monitor].width;
	int screen_height = server.monitor[panel->monitor].y + server.monitor[panel->monitor].height;
	if ( x+width <= screen_width && y+height <= screen_height && x>=server.monitor[panel->monitor].x && y>=server.monitor[panel->monitor].y )
		return;    // no adjustment needed

	if (panel_horizontal) {
		min_x=0;
		max_width=server.monitor[panel->monitor].width;
		max_height=server.monitor[panel->monitor].height-panel->area.bounds.height;
		if (panel_position & BOTTOM)
			min_y=0;
		else
			min_y=panel->area.bounds.height;
	}
	else {
		max_width=server.monitor[panel->monitor].width-panel->area.bounds.width;
		min_y=0;
		max_height=server.monitor[panel->monitor].height;
		if (panel_position & LEFT)
			min_x=panel->area.bounds.width;
		else
			min_x=0;
	}

	if (x+width > server.monitor[panel->monitor].x + server.monitor[panel->monitor].width)
		x = server.monitor[panel->monitor].x + server.monitor[panel->monitor].width - width;
	if ( y+height > server.monitor[panel->monitor].y + server.monitor[panel->monitor].height)
		y = server.monitor[panel->monitor].y + server.monitor[panel->monitor].height - height;

	if (x<min_x)
		x=min_x;
	if (width>max_width)
		width = max_width;
	if (y<min_y)
		y=min_y;
	if (height>max_height)
		height=max_height;
}

void tooltip_update()
{
	if (!g_tooltip.tooltip_text) {
		tooltip_hide(0);
		return;
	}

	tooltip_update_geometry();
	if (just_shown) {
		if (!panel_horizontal)
			y -= height/2; // center vertically
		just_shown = 0;
	}
	tooltip_adjust_geometry();
	XMoveResizeWindow(server.dsp, g_tooltip.window, x, y, width, height);

	// Stuff for drawing the tooltip
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout* layout;
	cs = cairo_xlib_surface_create(server.dsp, g_tooltip.window, server.visual, width, height);
	c = cairo_create(cs);
	/* Color bc = g_tooltip.background->back; */
  double bg_color[4];
  color_rgba_to_array (&g_tooltip.background->color, bg_color);
  border_t b = g_tooltip.background->border;
  if (server.real_transparency) {
    area_clear_pixmap (g_tooltip.window, rect_with_size (width,
							 height));

    progn {
      rectf_t rect = {
	.x = b.width,
	.y = b.width,
	.width = width - 2 * b.width,
	.height = height - 2 * b.width,
      };

      area_draw_rect (c, rect, b.radius - b.width / 1.571);
    }

  cairo_set_source_rgba (c, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	}
	else {
		cairo_rectangle(c, 0., 0, width, height);
  cairo_set_source_rgb (c, bg_color[0], bg_color[1], bg_color[2]);
	}
	cairo_fill(c);
	cairo_set_line_width(c, b.width);
	if (server.real_transparency)
     progn {
       rectf_t rect = {
	 .x = b.width / 2.0,
	 .y = b.width / 2.0,
	 .width = width - b.width,
	 .height = height - b.width,
       };
       area_draw_rect (c, rect, b.radius);
     }
	else
		cairo_rectangle(c, b.width/2.0, b.width/2.0, width-b.width, height-b.width);

  double color[4];
  color_rgba_to_array (&b.color, color);
  cairo_set_source_rgba (c, color[0], color[1], color[2], color[3]);
	cairo_stroke(c);

  double fc[4];
  color_rgba_to_array (&g_tooltip.font_color, fc);
  cairo_set_source_rgba (c, fc[0], fc[1], fc[2], fc[3]);
	layout = pango_cairo_create_layout(c);
	pango_layout_set_font_description(layout, g_tooltip.font_desc);
	pango_layout_set_text(layout, g_tooltip.tooltip_text, -1);
	PangoRectangle r1, r2;
	pango_layout_get_pixel_extents(layout, &r1, &r2);
	pango_layout_set_width(layout, width*PANGO_SCALE);
	pango_layout_set_height(layout, height*PANGO_SCALE);
	pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
	// I do not know why this is the right way, but with the below cairo_move_to it seems to be centered (horiz. and vert.)
	cairo_move_to(c,
				  -r1.x/2 + g_tooltip.background->border.width + g_tooltip.paddingx,
				  -r1.y/2 + 1 + g_tooltip.background->border.width + g_tooltip.paddingy);
	pango_cairo_show_layout (c, layout);

	g_object_unref (layout);
	cairo_destroy (c);
	cairo_surface_destroy (cs);
}


void tooltip_trigger_hide(Tooltip* tooltip) {
  UNUSED (tooltip);

	if (g_tooltip.mapped) {
		tooltip_copy_text(0);
		start_hide_timeout();
	}
	else {
		// tooltip not visible yet, but maybe a timeout is still pending
		stop_tooltip_timeout();
	}
}


void tooltip_hide(void* arg) {
  UNUSED (arg);
	if (g_tooltip.mapped) {
		g_tooltip.mapped = False;
		XUnmapWindow(server.dsp, g_tooltip.window);
		XFlush(server.dsp);
	}
}


void start_show_timeout()
{
	change_timeout(&g_tooltip.timeout, g_tooltip.show_timeout_msec, 0, tooltip_show, 0);
}


void start_hide_timeout()
{
	change_timeout(&g_tooltip.timeout, g_tooltip.hide_timeout_msec, 0, tooltip_hide, 0);
}


void stop_tooltip_timeout()
{
	stop_timeout(g_tooltip.timeout);
}


void tooltip_copy_text(Area* area)
{
	free(g_tooltip.tooltip_text);
	if (area && area->_get_tooltip_text)
		g_tooltip.tooltip_text = strdup(area->_get_tooltip_text(area));
	else
		g_tooltip.tooltip_text = NULL;
	g_tooltip.area = area;
}
