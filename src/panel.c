/**************************************************************************
*
* Copyright (C) 2008 Pål Staurland (staura@gmail.com)
* Modified (C) 2008 thierry lorthiois (lorthiois@bbsoft.fr) from Omega distribution
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
#include "conf.h" // For system checks.
#include "debug.h"

#include <stdio.h>
#include <stdint.h> // For `uint32_t'.
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>

#include "server.h"
#include "config.h"
#include "window.h"
#include "task.h"
#include "panel.h"
#include "tooltip.h"


// --------------------------------------------------
// mouse events
int mouse_left;
int mouse_middle;
int mouse_right;
int mouse_scroll_up;
int mouse_scroll_down;
int mouse_tilt_left;
int mouse_tilt_right;

int panel_mode;
int wm_menu;
int panel_dock;
int panel_layer;
int panel_position;
int panel_horizontal;
int panel_refresh;
int task_dragged;
char *panel_window_name = NULL;

int panel_autohide;
int panel_autohide_show_timeout;
int panel_autohide_hide_timeout;
int panel_autohide_height;
int panel_strut_policy;
char *panel_items_order;

int  max_tick_urgent;

// panel's initial config
Panel panel_config;
// panels (one panel per monitor)
Panel *panel1;
int  nb_panel;

GArray* backgrounds;

Imlib_Image default_icon;


static int  panel_resize (void *self);
static void panel_set_properties (Panel *p);
static void panel_init_size_and_position (Panel *panel);

void panel_default (void) {
	panel1 = 0;
	nb_panel = 0;
	default_icon = NULL;
	task_dragged = 0;
	panel_horizontal = 1;
	panel_position = CENTER;
	panel_items_order = NULL;
	panel_autohide = 0;
	panel_autohide_show_timeout = 0;
	panel_autohide_hide_timeout = 0;
	panel_autohide_height = 5;  // for vertical panels this is of course the width
	panel_strut_policy = STRUT_FOLLOW_SIZE;
	panel_dock = 0;  // default not in the dock
	panel_layer = BOTTOM_LAYER;  // default is bottom layer
	panel_window_name = strdup("tint2");
	wm_menu = 0;
	max_tick_urgent = 14;
	mouse_left = TOGGLE_ICONIFY;
	backgrounds = g_array_new(0, 0, sizeof(background_t));

	memset(&panel_config, 0, sizeof(Panel));

	// append full transparency background
	background_t transparent_bg;
	memset(&transparent_bg, 0, sizeof(background_t));
	g_array_append_val(backgrounds, transparent_bg);
}

void panel_cleanup (void) {
	if (!panel1)
		return;

	cleanup_taskbar();

	int i;
	Panel *p;
	for (i = 0; i < nb_panel; i++) {
		p = &panel1[i];

		area_destroy (&p->area);
		if (p->temp_pmap)
			XFreePixmap(server.dsp, p->temp_pmap);
		p->temp_pmap = 0;
		if (p->hidden_pixmap)
			XFreePixmap(server.dsp, p->hidden_pixmap);
		p->hidden_pixmap = 0;
		if (p->main_win)
			XDestroyWindow(server.dsp, p->main_win);
		p->main_win = 0;
		stop_timeout(p->autohide_timeout);
	}

	free(panel_items_order);
	panel_items_order = NULL;
	free(panel_window_name);
	panel_window_name = NULL;
	free(panel1);
	panel1 = NULL;
	if (backgrounds)
		g_array_free(backgrounds, 1);
	backgrounds = NULL;
	pango_font_description_free(panel_config.g_task.font_desc);
	panel_config.g_task.font_desc = NULL;
}

void panel_init (void) {
  int i;
	Panel *p;

	if (panel_config.monitor > (server.nb_monitor-1)) {
		// server.nb_monitor minimum value is 1 (see get_monitors())
		fprintf(stderr, "warning : monitor not found. tint2 default to all monitors.\n");
		panel_config.monitor = 0;
	}

	init_tooltip();
	init_systray();
	init_launcher();
	init_clock();
#ifdef ENABLE_BATTERY
	init_battery();
#endif
	init_taskbar();

	// number of panels (one monitor or 'all' monitors)
	if (panel_config.monitor >= 0)
		nb_panel = 1;
	else
		nb_panel = server.nb_monitor;

	panel1 = calloc(nb_panel, sizeof(Panel));
	for (i=0 ; i < nb_panel ; i++) {
		memcpy(&panel1[i], &panel_config, sizeof(Panel));
	}

	fprintf(stderr, "tint2 : nb monitor %d, nb monitor used %d, nb desktop %d\n", server.nb_monitor, nb_panel, server.nb_desktop);
	for (i=0 ; i < nb_panel ; i++) {
		p = &panel1[i];

		if (panel_config.monitor < 0)
			p->monitor = i;
		if (!p->area.background)
			p->area.background = &g_array_index(backgrounds, background_t, 0);
		p->area.parent = p;
		p->area.panel = p;
		p->area.visible = 1;
		p->area.resize = 1;
		p->area.size_mode = SIZE_BY_LAYOUT;
		p->area._resize = panel_resize;
		panel_init_size_and_position (p);
		// add childs according to panel_items
		for (size_t k = 0, len = strlen (panel_items_order); k < len ; ++k) {
			if (panel_items_order[k] == 'L')
				init_launcher_panel(p);
			else if (panel_items_order[k] == 'T')
				init_taskbar_panel(p);
#ifdef ENABLE_BATTERY
			else if (panel_items_order[k] == 'B')
				init_battery_panel(p);
#endif
			else if (panel_items_order[k] == 'S' && systray_on_monitor(i, nb_panel)) {
				init_systray_panel(p);
				refresh_systray = 1;
			}
			else if (panel_items_order[k] == 'C')
				init_clock_panel(p);
		}
		panel_set_items_order (p);

		// catch some events
		XSetWindowAttributes att = { .colormap=server.colormap, .background_pixel=0, .border_pixel=0 };
		unsigned long mask = CWEventMask|CWColormap|CWBackPixel|CWBorderPixel;
		p->main_win = XCreateWindow(server.dsp, server.root_win, p->posx, p->posy, p->area.bounds.width, p->area.bounds.height, 0, server.depth, InputOutput, server.visual, mask, &att);

		long event_mask = ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask;
		if (p->g_task.tooltip_enabled || p->clock.area._get_tooltip_text || (launcher_enabled && launcher_tooltip_enabled))
			event_mask |= PointerMotionMask|LeaveWindowMask;
		if (panel_autohide)
			event_mask |= LeaveWindowMask|EnterWindowMask;
		XChangeWindowAttributes(server.dsp, p->main_win, CWEventMask, &(XSetWindowAttributes){.event_mask=event_mask});

		if (!server.gc) {
			XGCValues  gcv;
			server.gc = XCreateGC(server.dsp, p->main_win, 0, &gcv);
		}
		//printf("panel %d : %d, %d, %d, %d\n", i, p->posx, p->posy, p->area.width, p->area.height);
		panel_set_properties (p);
		panel_set_background (p);
		if (snapshot_path == 0) {
			// if we are not in 'snapshot' mode then map new panel
			XMapWindow (server.dsp, p->main_win);
		}

		if (panel_autohide) panel_autohide_trigger_hide (p);

		visible_taskbar(p);
	}

	task_refresh_tasklist();
	active_task();
}


static void
panel_init_size_and_position (Panel *panel) {
	// detect panel size
	if (panel_horizontal) {
		if (panel->pourcentx)
			panel->area.bounds.width = (float)server.monitor[panel->monitor].width * panel->area.bounds.width / 100;
		if (panel->pourcenty)
			panel->area.bounds.height = (float)server.monitor[panel->monitor].height * panel->area.bounds.height / 100;
		if (panel->area.bounds.width + panel->marginx > server.monitor[panel->monitor].width)
			panel->area.bounds.width = server.monitor[panel->monitor].width - panel->marginx;
      if (panel->area.background->border.radius > panel->area.bounds.height / 2) {
			g_array_append_val(backgrounds, *panel->area.background);
			panel->area.background = &g_array_index(backgrounds, background_t, backgrounds->len-1);
        panel->area.background->border.radius = panel->area.bounds.height / 2;
      }
	}
	else {
		int old_panel_height = panel->area.bounds.height;
		if (panel->pourcentx)
			panel->area.bounds.height = (float)server.monitor[panel->monitor].height * panel->area.bounds.width / 100;
		else
			panel->area.bounds.height = panel->area.bounds.width;
		if (panel->pourcenty)
			panel->area.bounds.width = (float)server.monitor[panel->monitor].width * old_panel_height / 100;
		else
			panel->area.bounds.width = old_panel_height;
		if (panel->area.bounds.height + panel->marginy > server.monitor[panel->monitor].height)
			panel->area.bounds.height = server.monitor[panel->monitor].height - panel->marginy;
      if (panel->area.background->border.radius > panel->area.bounds.width / 2) {
			g_array_append_val(backgrounds, *panel->area.background);
			panel->area.background = &g_array_index(backgrounds, background_t, backgrounds->len-1);
        panel->area.background->border.radius = panel->area.bounds.width / 2;
		}
	}

	// panel position determined here
	if (panel_position & LEFT) {
		panel->posx = server.monitor[panel->monitor].x + panel->marginx;
	}
	else {
		if (panel_position & RIGHT) {
			panel->posx = server.monitor[panel->monitor].x + server.monitor[panel->monitor].width - panel->area.bounds.width - panel->marginx;
		}
		else {
			if (panel_horizontal)
				panel->posx = server.monitor[panel->monitor].x + ((server.monitor[panel->monitor].width - panel->area.bounds.width) / 2);
			else
				panel->posx = server.monitor[panel->monitor].x + panel->marginx;
		}
	}
	if (panel_position & TOP) {
		panel->posy = server.monitor[panel->monitor].y + panel->marginy;
	}
	else {
		if (panel_position & BOTTOM) {
			panel->posy = server.monitor[panel->monitor].y + server.monitor[panel->monitor].height - panel->area.bounds.height - panel->marginy;
		}
		else {
			panel->posy = server.monitor[panel->monitor].y + ((server.monitor[panel->monitor].height - panel->area.bounds.height) / 2);
		}
	}

	// autohide or strut_policy=minimum
	int diff = (panel_horizontal ? panel->area.bounds.height : panel->area.bounds.width) - panel_autohide_height;
	if (panel_horizontal) {
		panel->hidden_width = panel->area.bounds.width;
		panel->hidden_height = panel->area.bounds.height - diff;
	}
	else {
		panel->hidden_width = panel->area.bounds.width - diff;
		panel->hidden_height = panel->area.bounds.height;
	}
	// printf("panel : posx %d, posy %d, width %d, height %d\n", panel->posx, panel->posy, panel->area.width, panel->area.height);
}


static int panel_resize (void *obj)
{
	resize_by_layout(obj, 0);

	//printf("resize_panel\n");
	if (panel_mode != MULTI_DESKTOP && taskbar_enabled) {
		// propagate width/height on hidden taskbar
		int i, width, height;
		Panel *panel = (Panel*)obj;
		width = panel->taskbar[server.desktop].area.bounds.width;
		height = panel->taskbar[server.desktop].area.bounds.height;
		for (i=0 ; i < panel->nb_desktop ; i++) {
			panel->taskbar[i].area.bounds.width = width;
			panel->taskbar[i].area.bounds.height = height;
			panel->taskbar[i].area.resize = 1;
		}
	}
	if (panel_mode == MULTI_DESKTOP && taskbar_enabled && taskbar_distribute_size) {
		// Distribute the available space between taskbars
		Panel *panel = (Panel*)obj;

		// Compute the total available size, and the total size requested by the taskbars
		int total_size = 0;
		int total_name_size = 0;
		int total_items = 0;
		int i;
		for (i = 0; i < panel->nb_desktop; i++) {
			if (panel_horizontal) {
				total_size += panel->taskbar[i].area.bounds.width;
			} else {
				total_size += panel->taskbar[i].area.bounds.height;
			}

			Taskbar *taskbar = &panel->taskbar[i];
			GSList *l;
			for (l = taskbar->area.children; l; l = l->next) {
				Area *child = l->data;
				if (!child->visible)
					continue;
				total_items++;
			}
			if (taskbarname_enabled) {
				if (taskbar->area.children) {
					total_items--;
					Area *name = taskbar->area.children->data;
					if (panel_horizontal) {
						total_name_size += name->bounds.width;
					} else {
						total_name_size += name->bounds.height;
					}
				}
			}
		}
		// Distribute the space proportionally to the requested size (that is, to the
		// number of tasks in each taskbar)
		if (total_items) {
			int actual_name_size;
			if (total_name_size <= total_size) {
				actual_name_size = total_name_size / panel->nb_desktop;
			} else {
				actual_name_size = total_size / panel->nb_desktop;
			}
			total_size -= total_name_size;

			for (i = 0; i < panel->nb_desktop; i++) {
				Taskbar *taskbar = &panel->taskbar[i];

				int requested_size = (2 * taskbar->area.background->border.width) + (2 * taskbar->area.paddingxlr);
				int items = 0;
				GSList *l = taskbar->area.children;
				if (taskbarname_enabled)
					l = l->next;
				for (; l; l = l->next) {
					Area *child = l->data;
					if (!child->visible)
						continue;
					items++;
					if (panel_horizontal) {
						requested_size += child->bounds.width + taskbar->area.paddingy;
					} else {
						requested_size += child->bounds.height + taskbar->area.paddingx;
					}
				}
				if (panel_horizontal) {
					requested_size -= taskbar->area.paddingy;
				} else {
					requested_size -= taskbar->area.paddingx;
				}

				if (panel_horizontal) {
					taskbar->area.bounds.width = actual_name_size + items / (float)total_items * total_size;
				} else {
					taskbar->area.bounds.height = actual_name_size + items / (float)total_items * total_size;
				}
				taskbar->area.resize = 1;
			}
		}
	}
	return 0;
}


void update_strut(Panel* p)
{
	if (panel_strut_policy == STRUT_NONE) {
		XDeleteProperty(server.dsp, p->main_win, server.atom._NET_WM_STRUT);
		XDeleteProperty(server.dsp, p->main_win, server.atom._NET_WM_STRUT_PARTIAL);
		return;
	}

	// Reserved space
	unsigned int d1, screen_width, screen_height;
	Window d2;
	int d3;
	XGetGeometry(server.dsp, server.root_win, &d2, &d3, &d3, &screen_width, &screen_height, &d1, &d1);
	Monitor monitor = server.monitor[p->monitor];
	long   struts [12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (panel_horizontal) {
		int height = p->area.bounds.height + p->marginy;
		if (panel_strut_policy == STRUT_MINIMUM || (panel_strut_policy == STRUT_FOLLOW_SIZE && p->is_hidden))
			height = p->hidden_height;
		if (panel_position & TOP) {
			struts[2] = height + monitor.y;
			struts[8] = p->posx;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[9] = p->posx + p->area.bounds.width - 1;
		}
		else {
			struts[3] = height + screen_height - monitor.y - monitor.height;
			struts[10] = p->posx;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[11] = p->posx + p->area.bounds.width - 1;
		}
	}
	else {
		int width = p->area.bounds.width + p->marginx;
		if (panel_strut_policy == STRUT_MINIMUM || (panel_strut_policy == STRUT_FOLLOW_SIZE && p->is_hidden))
			width = p->hidden_width;
		if (panel_position & LEFT) {
			struts[0] = width + monitor.x;
			struts[4] = p->posy;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[5] = p->posy + p->area.bounds.height - 1;
		}
		else {
			struts[1] = width + screen_width - monitor.x - monitor.width;
			struts[6] = p->posy;
			// p->area.width - 1 allowed full screen on monitor 2
			struts[7] = p->posy + p->area.bounds.height - 1;
		}
	}
	// Old specification : fluxbox need _NET_WM_STRUT.
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_STRUT, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &struts, 4);
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_STRUT_PARTIAL, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &struts, 12);
}

void panel_set_items_order (Panel *p) {
	int j;

	if (p->area.children) {
		g_slist_free(p->area.children);
		p->area.children = 0;
	}

	for (uint32_t k = 0, len = strlen (panel_items_order); k < len; ++k) {
		if (panel_items_order[k] == 'L') {
			p->area.children = g_slist_append(p->area.children, &p->launcher);
			p->launcher.area.resize = 1;
		}
		else if (panel_items_order[k] == 'T') {
			for (j=0 ; j < p->nb_desktop ; j++)
				p->area.children = g_slist_append(p->area.children, &p->taskbar[j]);
		}
#ifdef ENABLE_BATTERY
		else if (panel_items_order[k] == 'B')
			p->area.children = g_slist_append(p->area.children, &p->battery);
#endif
	        else if (panel_items_order[k] == 'S') {
		  int i = p - panel1;
		  if (systray_on_monitor(i, nb_panel))
		    p->area.children = g_slist_append(p->area.children, &systray);
		}
		else if (panel_items_order[k] == 'C')
			p->area.children = g_slist_append(p->area.children, &p->clock);
	}
	init_rendering(&p->area, 0);
}


static void panel_set_properties (Panel *p) {
	XStoreName (server.dsp, p->main_win, panel_window_name);
	XSetIconName (server.dsp, p->main_win, panel_window_name);

	gsize len;
	gchar *name = g_locale_to_utf8(panel_window_name, -1, NULL, &len, NULL);
	if (name) {
		XChangeProperty(server.dsp, p->main_win, server.atom._NET_WM_NAME, server.atom.UTF8_STRING, 8, PropModeReplace, (unsigned char *) name, (int) len);
		XChangeProperty(server.dsp, p->main_win, server.atom._NET_WM_ICON_NAME, server.atom.UTF8_STRING, 8, PropModeReplace, (unsigned char *) name, (int) len);
		g_free(name);
	}

	// Dock
	long val = panel_dock ? server.atom._NET_WM_WINDOW_TYPE_DOCK : server.atom._NET_WM_WINDOW_TYPE_SPLASH;
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char *) &val, 1);

	val = ALLDESKTOP;
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_DESKTOP, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &val, 1);

	Atom state[] = {
	  server.atom._NET_WM_STATE_SKIP_PAGER,
	  server.atom._NET_WM_STATE_SKIP_TASKBAR,
	  server.atom._NET_WM_STATE_STICKY,
	  panel_layer == BOTTOM_LAYER ? server.atom._NET_WM_STATE_BELOW : server.atom._NET_WM_STATE_ABOVE,
	};

	int nb_atoms = panel_layer == NORMAL_LAYER ? 3 : 4;
	XChangeProperty (server.dsp, p->main_win, server.atom._NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char *) state, nb_atoms);

	XWMHints wmhints;
	memset(&wmhints, 0, sizeof(wmhints));
	if (panel_dock) {
		// Necessary for placing the panel into the dock on Openbox and Fluxbox.
		// See https://code.google.com/p/tint2/issues/detail?id=465
		wmhints.icon_window = wmhints.window_group = p->main_win;
		wmhints.flags = StateHint | IconWindowHint;
		wmhints.initial_state = WithdrawnState;
	}
	// We do not need keyboard input focus.
	wmhints.flags |= InputHint;
	wmhints.input = False;
	XSetWMHints(server.dsp, p->main_win, &wmhints);

	// Undecorated
	long prop[5] = { 2, 0, 0, 0, 0 };
	XChangeProperty(server.dsp, p->main_win, server.atom._MOTIF_WM_HINTS, server.atom._MOTIF_WM_HINTS, 32, PropModeReplace, (unsigned char *) prop, 5);

	// XdndAware - Register for Xdnd events
	Atom version=4;
	XChangeProperty(server.dsp, p->main_win, server.atom.XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&version, 1);

	update_strut(p);

	// Fixed position and non-resizable window
	// Allow panel move and resize when tint2 reload config file
	int minwidth = panel_autohide ? p->hidden_width : p->area.bounds.width;
	int minheight = panel_autohide ? p->hidden_height : p->area.bounds.height;
	XSizeHints size_hints = {
	  .flags = PPosition | PMinSize | PMaxSize,
	  .min_width = minwidth,
	  .max_width = p->area.bounds.width,
	  .min_height = minheight,
	  .max_height = p->area.bounds.height,
	};

	XSetWMNormalHints(server.dsp, p->main_win, &size_hints);

	// Set WM_CLASS
	XClassHint* classhint = XAllocClassHint();
	classhint->res_name = PROJECT_NAME;
	classhint->res_class = PROJECT_NAME;
	XSetClassHint(server.dsp, p->main_win, classhint);
	XFree(classhint);
}


void panel_set_background (Panel *p)
{
	if (p->area.pixmap) XFreePixmap (server.dsp, p->area.pixmap);
	p->area.pixmap = XCreatePixmap (server.dsp, server.root_win, p->area.bounds.width, p->area.bounds.height, server.depth);

	int xoff=0, yoff=0;
	if (panel_horizontal && panel_position & BOTTOM)
		yoff = p->area.bounds.height-p->hidden_height;
	else if (!panel_horizontal && panel_position & RIGHT)
		xoff = p->area.bounds.width-p->hidden_width;

	if (server.real_transparency) {
	  area_clear_pixmap (p->area.pixmap,
			    rect_with_size (p->area.bounds.width,
					    p->area.bounds.height));
	}
	else {
		get_root_pixmap();
		// copy background (server.root_pmap) in panel.area.pixmap
		Window dummy;
		int  x, y;
		XTranslateCoordinates(server.dsp, p->main_win, server.root_win, 0, 0, &x, &y, &dummy);
		if (panel_autohide && p->is_hidden) {
			x -= xoff;
			y -= yoff;
		}
		XSetTSOrigin(server.dsp, server.gc, -x, -y);
		XFillRectangle(server.dsp, p->area.pixmap, server.gc, 0, 0, p->area.bounds.width, p->area.bounds.height);
	}

	// draw background panel
	cairo_surface_t *cs = cairo_xlib_surface_create (server.dsp, p->area.pixmap, server.visual, p->area.bounds.width, p->area.bounds.height);;
	cairo_t *c  = cairo_create (cs);;

	draw_background(&p->area, c);
	cairo_destroy (c);
	cairo_surface_destroy (cs);

	if (panel_autohide) {
		if (p->hidden_pixmap) XFreePixmap(server.dsp, p->hidden_pixmap);
		p->hidden_pixmap = XCreatePixmap(server.dsp, server.root_win, p->hidden_width, p->hidden_height, server.depth);
		XCopyArea(server.dsp, p->area.pixmap, p->hidden_pixmap, server.gc, xoff, yoff, p->hidden_width, p->hidden_height, 0, 0);
	}

	// redraw panel's object
	GSList *l0;
	Area *a;
	for (l0 = p->area.children; l0 ; l0 = l0->next) {
		a = l0->data;
		set_redraw(a);
	}

	// reset task/taskbar 'state_pix'
	int i, k;
	Taskbar *tskbar;
	for (i=0 ; i < p->nb_desktop ; i++) {
		tskbar = &p->taskbar[i];
		for (k=0; k<TASKBAR_STATE_COUNT; ++k) {
			if (tskbar->state_pix[k]) XFreePixmap(server.dsp, tskbar->state_pix[k]);
			tskbar->state_pix[k] = 0;
			if (tskbar->bar_name.state_pix[k]) XFreePixmap(server.dsp, tskbar->bar_name.state_pix[k]);
			tskbar->bar_name.state_pix[k] = 0;
		}
		tskbar->area.pixmap = 0;
		tskbar->bar_name.area.pixmap = 0;
		l0 = tskbar->area.children;
		if (taskbarname_enabled) l0 = l0->next;
		for (; l0 ; l0 = l0->next) {
			set_task_redraw((Task *)l0->data);
		}
	}
}


Panel* panel_get (Window win) {

	for (int i = 0 ; i < nb_panel ; ++i) {
		if (panel1[i].main_win == win) {
			return panel1 + i;
		}
	}
	return 0;
}


Taskbar *panel_click_taskbar (Panel *panel, point_t point) {
  Taskbar *tskbar = NULL;

	if (panel_horizontal) {
		for (int i=0; i < panel->nb_desktop ; ++i) {
		  tskbar = panel->taskbar + i;
			if (tskbar->area.visible && point.x >= tskbar->area.bounds.x && point.x <= (tskbar->area.bounds.x + tskbar->area.bounds.width))
				return tskbar;
		}
	}
	else {
		for (int i = 0; i < panel->nb_desktop ; ++i) {
		  tskbar = panel->taskbar + i;
			if (tskbar->area.visible && point.y >= tskbar->area.bounds.y && point.y <= (tskbar->area.bounds.y + tskbar->area.bounds.height))
				return tskbar;
		}
	}
	return NULL;
}


Task* panel_click_task (Panel *panel, point_t point)
{
	GSList *l0;

  Taskbar* tskbar = panel_click_taskbar (panel, point);
	if (tskbar) {
		if (panel_horizontal) {
			Task *tsk;
			l0 = tskbar->area.children;
			if (taskbarname_enabled) l0 = l0->next;
			for (; l0 ; l0 = l0->next) {
				tsk = l0->data;
				if (tsk->area.visible && point.x >= tsk->area.bounds.x && point.x <= (tsk->area.bounds.x + tsk->area.bounds.width)) {
					return tsk;
				}
			}
		}
		else {
			Task *tsk;
			l0 = tskbar->area.children;
			if (taskbarname_enabled) l0 = l0->next;
			for (; l0 ; l0 = l0->next) {
				tsk = l0->data;
				if (tsk->area.visible && point.y >= tsk->area.bounds.y && point.y <= (tsk->area.bounds.y + tsk->area.bounds.height)) {
					return tsk;
				}
			}
		}
	}
	return NULL;
}


Launcher *panel_click_launcher (Panel *panel, point_t point)
{
	Launcher *launcher = &panel->launcher;

	if (panel_horizontal) {
		if (launcher->area.visible && point.x >= launcher->area.bounds.x
		    && point.x <= (launcher->area.bounds.x + launcher->area.bounds.width)) {
		  return launcher;
		}
	}
	else {
		if (launcher->area.visible && point.y >= launcher->area.bounds.y
		    && point.y <= (launcher->area.bounds.y + launcher->area.bounds.height)) {
		  return launcher;
		}
	}
	return NULL;
}


LauncherIcon *panel_click_launcher_icon (Panel *panel, point_t point)
{
	GSList *l0;
	Launcher *launcher;

	if ( (launcher = panel_click_launcher (panel, point)) ) {
		LauncherIcon *icon;
		for (l0 = launcher->list_icons; l0 ; l0 = l0->next) {
			icon = l0->data;
			if (point.x >= (launcher->area.bounds.x + icon->x)
			    && point.x <= (launcher->area.bounds.x + icon->x + icon->icon_size) &&
			    point.y >= (launcher->area.bounds.y + icon->y)
			    && point.y <= (launcher->area.bounds.y + icon->y + icon->icon_size)) {
			  return icon;
			}
		}
	}
	return NULL;
}


// NOTE: This function is not being used by anyone.
int panel_click_padding (Panel *panel, point_t point)
{
	if (panel_horizontal) {
		if (point.x < panel->area.paddingxlr || point.x > panel->area.bounds.width-panel->area.paddingxlr)
		  return 1;
	}
	else {
		if (point.y < panel->area.paddingxlr || point.y > panel->area.bounds.height-panel->area.paddingxlr)
		  return 1;
	}
	return 0;
}


int panel_click_clock (Panel *panel, point_t point) {
	Clock clk = panel->clock;
	if (panel_horizontal) {
		if (clk.area.visible && point.x >= clk.area.bounds.x &&
		    point.x <= (clk.area.bounds.x + clk.area.bounds.width)) {
		  return TRUE;
		}
	} else {
		if (clk.area.visible && point.y >= clk.area.bounds.y
		    && point.y <= (clk.area.bounds.y + clk.area.bounds.height)) {
		  return TRUE;
		}
	}
	return FALSE;
}


Area* panel_click_area (Panel *panel, point_t point) {
	Area* result = &panel->area;
	Area* new_result = result;
	do {
		result = new_result;
		GSList* it = result->children;
		while (it) {
			Area* a = it->data;
			if (a->visible && point.x >= a->bounds.x && point.x <= (a->bounds.x + a->bounds.width)
					&& point.y >= a->bounds.y && point.y <= (a->bounds.y + a->bounds.height)) {
				new_result = a;
				break;
			}
			it = it->next;
		}
	} while (new_result != result);
	return result;
}


void stop_autohide_timeout(Panel* p)
{
	stop_timeout(p->autohide_timeout);
}


void panel_autohide_show(void* p)
{
	Panel* panel = p;
	stop_autohide_timeout(panel);
	panel->is_hidden = 0;

	XMapSubwindows(server.dsp, panel->main_win);  // systray windows
	if (panel_horizontal) {
		if (panel_position & TOP)
			XResizeWindow(server.dsp, panel->main_win, panel->area.bounds.width, panel->area.bounds.height);
		else
			XMoveResizeWindow(server.dsp, panel->main_win, panel->posx, panel->posy, panel->area.bounds.width, panel->area.bounds.height);
	}
	else {
		if (panel_position & LEFT)
			XResizeWindow(server.dsp, panel->main_win, panel->area.bounds.width, panel->area.bounds.height);
		else
			XMoveResizeWindow(server.dsp, panel->main_win, panel->posx, panel->posy, panel->area.bounds.width, panel->area.bounds.height);
	}
	if (panel_strut_policy == STRUT_FOLLOW_SIZE)
		update_strut(p);
	refresh_systray = 1;   // ugly hack, because we actually only need to call XSetbackground_tPixmap
	panel_refresh = 1;
}


void autohide_hide(void* p)
{
	Panel* panel = p;
	stop_autohide_timeout(panel);
	panel->is_hidden = 1;
	if (panel_strut_policy == STRUT_FOLLOW_SIZE)
		update_strut(p);

	XUnmapSubwindows(server.dsp, panel->main_win);  // systray windows
	int diff = (panel_horizontal ? panel->area.bounds.height : panel->area.bounds.width) - panel_autohide_height;
	//printf("autohide_hide : diff %d, w %d, h %d\n", diff, panel->hidden_width, panel->hidden_height);
	if (panel_horizontal) {
		if (panel_position & TOP)
			XResizeWindow(server.dsp, panel->main_win, panel->hidden_width, panel->hidden_height);
		else
			XMoveResizeWindow(server.dsp, panel->main_win, panel->posx, panel->posy+diff, panel->hidden_width, panel->hidden_height);
	}
	else {
		if (panel_position & LEFT)
			XResizeWindow(server.dsp, panel->main_win, panel->hidden_width, panel->hidden_height);
		else
			XMoveResizeWindow(server.dsp, panel->main_win, panel->posx+diff, panel->posy, panel->hidden_width, panel->hidden_height);
	}
	panel_refresh = 1;
}


void panel_autohide_trigger_show (Panel* p) {
	if (!p)
		return;
	change_timeout(&p->autohide_timeout, panel_autohide_show_timeout, 0, panel_autohide_show, p);
}


void panel_autohide_trigger_hide (Panel* p) {
	if (!p)
		return;

	Window root, child;
	int xr, yr, xw, yw;
	unsigned int mask;
	if (XQueryPointer(server.dsp, p->main_win, &root, &child, &xr, &yr, &xw, &yw, &mask))
		if (child) return;  // mouse over one of the system tray icons

	change_timeout(&p->autohide_timeout, panel_autohide_hide_timeout, 0, autohide_hide, p);
}
