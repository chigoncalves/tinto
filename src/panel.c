/**************************************************************************
*
* Copyright (C) 2008 Pål Staurland (staura@gmail.com)
* Modified (C) 2008 thierry lorthiois (lorthiois@bbsoft.fr)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>

#include "server.h"
#include "window.h"
#include "task.h"
#include "panel.h"
#include "tooltip.h"


int signal_pending;
// --------------------------------------------------
// mouse events
int mouse_middle;
int mouse_right;
int mouse_scroll_up;
int mouse_scroll_down;
int mouse_tilt_left;
int mouse_tilt_right;

int panel_mode;
int wm_menu;
int panel_dock=0;  // default not in the dock
int panel_position;
int panel_horizontal;
int panel_refresh;

Task *task_active;
Task *task_drag;
Task *task_urgent;
int  tick_urgent;
int  max_tick_urgent;

// panel's initial config
Panel panel_config;
// panels (one panel per monitor)
Panel *panel1 = NULL;
int  nb_panel = 0;

Imlib_Image default_icon = NULL;



void init_panel()
{
	int i, old_nb_panel;
	Panel *new_panel, *p;

	cleanup_taskbar();
	for (i=0 ; i < nb_panel ; i++) {
		free_area(&panel1[i].area);
		if (panel1[i].temp_pmap) {
			XFreePixmap(server.dsp, panel1[i].temp_pmap);
			panel1[i].temp_pmap = 0;
		}
	}

	// alloc panels (one monitor or all monitors)
	old_nb_panel = nb_panel;
	if (panel_config.monitor >= 0)
		nb_panel = 1;
	else
		nb_panel = server.nb_monitor;

	if (nb_panel < old_nb_panel) {
		// freed old panels
		for (i=nb_panel ; i < old_nb_panel ; i++) {
			if (panel1[i].main_win)
				XDestroyWindow(server.dsp, panel1[i].main_win);
		}
		new_panel = realloc(panel1, nb_panel * sizeof(Panel));
	}
	else if (nb_panel > old_nb_panel) {
		new_panel = realloc(panel1, nb_panel * sizeof(Panel));
		// init new panel
		for (i=old_nb_panel ; i < nb_panel ; i++) {
			memcpy(&new_panel[i], &panel_config, sizeof(Panel));
		}
	}
	else
		new_panel = panel1;

	fprintf(stderr, "tint2 : nb monitor %d, nb desktop %d\n", nb_panel, server.nb_desktop);
	for (i=0 ; i < nb_panel ; i++) {
		p = &new_panel[i];

		p->monitor = i;
		p->area.parent = p;
		p->area.panel = p;
		p->area.on_screen = 1;
		p->area.resize = 1;
		p->area._resize = resize_panel;
		p->g_taskbar.parent = p;
		p->g_taskbar.panel = p;
		p->g_task.area.panel = p;
		init_panel_size_and_position(p);

		// add childs
		if (p->clock.area.on_screen) {
			init_clock_panel(p);
			p->area.list = g_slist_append(p->area.list, &p->clock);
		}
#ifdef ENABLE_BATTERY
		if (p->battery.area.on_screen) {
			init_battery_panel(p);
			p->area.list = g_slist_append(p->area.list, &p->battery);
		}
#endif
		// systray only on first panel
		if (systray.area.on_screen && i == 0) {
			init_systray_panel(p);
			p->area.list = g_slist_append(p->area.list, &systray);
		}

		// full width mode
		if (!p->initial_width) {
			p->initial_width = 100;
			p->pourcentx = 1;
		}

		if (i >= old_nb_panel) {
			// new panel
			printf("new panel %d : %d, %d, %d, %d\n", i, p->posx, p->posy, p->area.width, p->area.height);

			// Catch some events
			long event_mask = ExposureMask|ButtonPressMask|ButtonReleaseMask;
			if (g_tooltip.enabled)
				event_mask |= PointerMotionMask|LeaveWindowMask;
			XSetWindowAttributes att = { ParentRelative, 0L, 0, 0L, 0, 0, Always, 0L, 0L, False, event_mask, NoEventMask, False, 0, 0 };
			if (p->main_win) XDestroyWindow(server.dsp, p->main_win);
			p->main_win = XCreateWindow(server.dsp, server.root_win, p->posx, p->posy, p->area.width, p->area.height, 0, server.depth, InputOutput, CopyFromParent, CWEventMask, &att);

			set_panel_properties(p);
			set_panel_background(p);
			XMapWindow (server.dsp, p->main_win);
		}
		else {
			// old panel
			printf("old panel %d : %d, %d, %d, %d\n", i, p->posx, p->posy, p->area.width, p->area.height);
			XMoveResizeWindow(server.dsp, p->main_win, p->posx, p->posy, p->area.width, p->area.height);
			set_panel_background(p);
		}
	}

	panel1 = new_panel;
	panel_refresh = 1;
	refresh_systray = 1;
	init_taskbar();
	visible_object();
	task_refresh_tasklist();
}


void init_panel_size_and_position(Panel *panel)
{
	// detect panel size
	if (panel_horizontal) {
		if (panel->pourcentx)
			panel->area.width = (float)server.monitor[panel->monitor].width * panel->initial_width / 100;
		else
			panel->area.width = panel->initial_width;
		if (panel->pourcenty)
			panel->area.height = (float)server.monitor[panel->monitor].height * panel->initial_height / 100;
		else
			panel->area.height = panel->initial_height;
		if (panel->area.pix.border.rounded > panel->area.height/2)
			panel->area.pix.border.rounded = panel->area.height/2;
	}
	else {
		if (panel->pourcentx)
			panel->area.height = (float)server.monitor[panel->monitor].height * panel->initial_width / 100;
		else
			panel->area.height = panel->initial_width;
		if (panel->pourcenty)
			panel->area.width = (float)server.monitor[panel->monitor].width * panel->initial_height / 100;
		else
			panel->area.width = panel->initial_height;
		if (panel->area.pix.border.rounded > panel->area.width/2)
			panel->area.pix.border.rounded = panel->area.width/2;
	}

	// panel position determined here
	if (panel_position & LEFT) {
		panel->posx = server.monitor[panel->monitor].x + panel->marginx;
	}
	else {
		if (panel_position & RIGHT) {
			panel->posx = server.monitor[panel->monitor].x + server.monitor[panel->monitor].width - panel->area.width - panel->marginx;
		}
		else {
			if (panel_horizontal)
				panel->posx = server.monitor[panel->monitor].x + ((server.monitor[panel->monitor].width - panel->area.width) / 2);
			else
				panel->posx = server.monitor[panel->monitor].x + panel->marginx;
		}
	}
	if (panel_position & TOP) {
		panel->posy = server.monitor[panel->monitor].y + panel->marginy;
	}
	else {
		if (panel_position & BOTTOM) {
			panel->posy = server.monitor[panel->monitor].y + server.monitor[panel->monitor].height - panel->area.height - panel->marginy;
		}
		else {
			panel->posy = server.monitor[panel->monitor].y + ((server.monitor[panel->monitor].height - panel->area.height) / 2);
		}
	}
	// printf("panel : posx %d, posy %d, width %d, height %d\n", panel->posx, panel->posy, panel->area.width, panel->area.height);
}


void cleanup_panel()
{
	if (!panel1) return;

	task_active = 0;
	task_drag = 0;
	task_urgent = 0;
	cleanup_taskbar();

	// font allocated once
	if (panel1[0].g_task.font_desc) {
		pango_font_description_free(panel1[0].g_task.font_desc);
		panel1[0].g_task.font_desc = 0;
	}

	int i;
	Panel *p;
	for (i=0 ; i < nb_panel ; i++) {
		p = &panel1[i];

		free_area(&p->area);

		if (p->temp_pmap) {
			XFreePixmap(server.dsp, p->temp_pmap);
			p->temp_pmap = 0;
		}
		if (p->main_win) {
			XDestroyWindow(server.dsp, p->main_win);
			p->main_win = 0;
		}
	}

	if (panel1) free(panel1);
	panel1 = 0;
}


void resize_panel(void *obj)
{
	Panel *panel = (Panel*)obj;

	if (panel_horizontal) {
		int taskbar_width, modulo_width = 0;

		taskbar_width = panel->area.width - (2 * panel->area.paddingxlr) - (2 * panel->area.pix.border.width);
		if (panel->clock.area.on_screen && panel->clock.area.width)
			taskbar_width -= (panel->clock.area.width + panel->area.paddingx);
	#ifdef ENABLE_BATTERY
		if (panel->battery.area.on_screen && panel->battery.area.width)
			taskbar_width -= (panel->battery.area.width + panel->area.paddingx);
	#endif
		// TODO : systray only on first panel. search better implementation !
		if (systray.area.on_screen && systray.area.width && panel == &panel1[0])
			taskbar_width -= (systray.area.width + panel->area.paddingx);

		if (panel_mode == MULTI_DESKTOP) {
			int width = taskbar_width - ((panel->nb_desktop-1) * panel->area.paddingx);
			taskbar_width = width / panel->nb_desktop;
			modulo_width = width % panel->nb_desktop;
		}

		// change posx and width for all taskbar
		int i, posx;
		posx = panel->area.pix.border.width + panel->area.paddingxlr;
		for (i=0 ; i < panel->nb_desktop ; i++) {
			panel->taskbar[i].area.posx = posx;
			panel->taskbar[i].area.width = taskbar_width;
			panel->taskbar[i].area.resize = 1;
			if (modulo_width) {
				panel->taskbar[i].area.width++;
				modulo_width--;
			}
			//printf("taskbar %d : posx %d, width, %d, posy %d\n", i, posx, panel->taskbar[i].area.width, posx + panel->taskbar[i].area.width);
			if (panel_mode == MULTI_DESKTOP)
				posx += panel->taskbar[i].area.width + panel->area.paddingx;
		}
	}
	else {
		int taskbar_height, modulo_height = 0;
		int i, posy;

		taskbar_height = panel->area.height - (2 * panel->area.paddingxlr) - (2 * panel->area.pix.border.width);
		if (panel->clock.area.on_screen && panel->clock.area.height)
			taskbar_height -= (panel->clock.area.height + panel->area.paddingx);
	#ifdef ENABLE_BATTERY
		if (panel->battery.area.on_screen && panel->battery.area.height)
			taskbar_height -= (panel->battery.area.height + panel->area.paddingx);
	#endif
		// TODO : systray only on first panel. search better implementation !
		if (systray.area.on_screen && systray.area.height && panel == &panel1[0])
			taskbar_height -= (systray.area.height + panel->area.paddingx);

		posy = panel->area.height - panel->area.pix.border.width - panel->area.paddingxlr - taskbar_height;
		if (panel_mode == MULTI_DESKTOP) {
			int height = taskbar_height - ((panel->nb_desktop-1) * panel->area.paddingx);
			taskbar_height = height / panel->nb_desktop;
			modulo_height = height % panel->nb_desktop;
		}

		// change posy and height for all taskbar
		for (i=0 ; i < panel->nb_desktop ; i++) {
			panel->taskbar[i].area.posy = posy;
			panel->taskbar[i].area.height = taskbar_height;
			panel->taskbar[i].area.resize = 1;
			if (modulo_height) {
				panel->taskbar[i].area.height++;
				modulo_height--;
			}
			if (panel_mode == MULTI_DESKTOP)
				posy += panel->taskbar[i].area.height + panel->area.paddingx;
		}
	}
}


void visible_object()
{
	Panel *panel;
	int i, j;

	for (i=0 ; i < nb_panel ; i++) {
		panel = &panel1[i];

		Taskbar *taskbar;
		for (j=0 ; j < panel->nb_desktop ; j++) {
			taskbar = &panel->taskbar[j];
			if (panel_mode != MULTI_DESKTOP && taskbar->desktop != server.desktop) {
				// SINGLE_DESKTOP and not current desktop
				taskbar->area.on_screen = 0;
			}
			else {
				taskbar->area.on_screen = 1;
			}
		}
	}
	panel_refresh = 1;
}


void set_panel_properties(Panel *p)
{
	XStoreName (server.dsp, p->main_win, "tint2");

	gsize len;
	gchar *name = g_locale_to_utf8("tint2", -1, NULL, &len, NULL);
	if (name != NULL) {
		XChangeProperty(server.dsp, p->main_win, server.atom._NET_WM_NAME, server.atom.UTF8_STRING, 8, PropModeReplace, (unsigned char *) name, (int) len);
		g_free(name);
	}

	// Dock
	long val = server.atom._NET_WM_WINDOW_TYPE_DOCK;
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char *) &val, 1);

	// Reserved space
	long   struts [12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (panel_horizontal) {
		if (panel_position & TOP) {
			struts[2] = p->area.height + p->marginy;
			struts[8] = p->posx;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[9] = p->posx + p->area.width - 1;
		}
		else {
			struts[3] = p->area.height + p->marginy;
			struts[10] = p->posx;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[11] = p->posx + p->area.width - 1;
		}
	}
	else {
		if (panel_position & LEFT) {
			struts[0] = p->area.width + p->marginx;
			struts[4] = p->posy;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[5] = p->posy + p->area.height - 1;
		}
		else {
			struts[1] = p->area.width + p->marginx;
			struts[6] = p->posy;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[7] = p->posy + p->area.height - 1;
		}
	}
	// Old specification : fluxbox need _NET_WM_STRUT.
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_STRUT, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &struts, 4);
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_STRUT_PARTIAL, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &struts, 12);

	// Sticky and below other window
	val = 0xFFFFFFFF;
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_DESKTOP, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &val, 1);
	Atom state[4];
	state[0] = server.atom._NET_WM_STATE_SKIP_PAGER;
	state[1] = server.atom._NET_WM_STATE_SKIP_TASKBAR;
	state[2] = server.atom._NET_WM_STATE_STICKY;
	state[3] = server.atom._NET_WM_STATE_BELOW;
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char *) state, 4);

	// Fixed position and non-resizable window
	XSizeHints size_hints;
	size_hints.flags = PPosition|PMinSize|PMaxSize;
	size_hints.min_width = size_hints.max_width = p->area.width;
	size_hints.min_height = size_hints.max_height = p->area.height;
	XSetWMNormalHints(server.dsp, p->main_win, &size_hints);

	// Unfocusable
	XWMHints wmhints;
	if (panel_dock) {
		// TODO: Xdnd feature cannot be used in withdrawn state at the moment (at least GTK apps fail, qt seems to work)
		wmhints.icon_window = wmhints.window_group = p->main_win;
		wmhints.flags = StateHint | IconWindowHint;
		wmhints.initial_state = WithdrawnState;
	}
	else {
		wmhints.flags = InputHint;
		wmhints.input = False;
	}
	XSetWMHints(server.dsp, p->main_win, &wmhints);

	// Undecorated
	long prop[5] = { 2, 0, 0, 0, 0 };
	XChangeProperty(server.dsp, p->main_win, server.atom._MOTIF_WM_HINTS, server.atom._MOTIF_WM_HINTS, 32, PropModeReplace, (unsigned char *) prop, 5);

	// XdndAware - Register for Xdnd events
	int version=5;
	XChangeProperty(server.dsp, p->main_win, server.atom.XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&version, 1);
}


void set_panel_background(Panel *p)
{
	get_root_pixmap();

	if (p->area.pix.pmap) XFreePixmap (server.dsp, p->area.pix.pmap);
	p->area.pix.pmap = XCreatePixmap (server.dsp, server.root_win, p->area.width, p->area.height, server.depth);

	// copy background (server.root_pmap) in panel.area.pix.pmap
	Window dummy;
	int  x, y;
	XTranslateCoordinates(server.dsp, p->main_win, server.root_win, 0, 0, &x, &y, &dummy);
	XSetTSOrigin(server.dsp, server.gc, -x, -y) ;
	XFillRectangle(server.dsp, p->area.pix.pmap, server.gc, 0, 0, p->area.width, p->area.height);

	// draw background panel
	cairo_surface_t *cs;
	cairo_t *c;
	cs = cairo_xlib_surface_create (server.dsp, p->area.pix.pmap, server.visual, p->area.width, p->area.height);
	c = cairo_create (cs);

	draw_background(&p->area, c, 0);

	cairo_destroy (c);
	cairo_surface_destroy (cs);

	// redraw panel's object
	GSList *l0;
	Area *a;
	for (l0 = p->area.list; l0 ; l0 = l0->next) {
		a = l0->data;
		set_redraw(a);
	}
}


Panel *get_panel(Window win)
{
	int i;
	for (i=0 ; i < nb_panel ; i++) {
		if (panel1[i].main_win == win) {
			return &panel1[i];
		}
	}
	return 0;
}

