/**************************************************************************
* Tint2 : systraybar
*
* Copyright (C) 2009 thierry lorthiois (lorthiois@bbsoft.fr)
* based on 'docker-1.5' from Ben Jansens.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <Imlib2.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>

#include "systraybar.h"
#include "server.h"
#include "panel.h"

GSList *icons;

/* defined in the systray spec */
#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

// selection window
Window net_sel_win = None, hint_win = None;

// freedesktop specification doesn't allow multi systray
Systraybar systray;
int refresh_systray;
int systray_enabled;


void init_systray()
{
	start_net();

	if (!systray_enabled)
		return;

	systray.area._draw_foreground = draw_systray;
	systray.area._resize = resize_systray;
	systray.area.resize = 1;
	systray.area.redraw = 1;
	systray.area.on_screen = 1;
	refresh_systray = 0;
}


void init_systray_panel(void *p)
{
	Panel *panel =(Panel*)p;

	if (panel_horizontal) {
		systray.area.posy = panel->area.pix.border.width + panel->area.paddingy;
		systray.area.height = panel->area.height - (2 * systray.area.posy);
	}
	else {
		systray.area.posx = panel->area.pix.border.width + panel->area.paddingy;
		systray.area.width = panel->area.width - (2 * panel->area.pix.border.width) - (2 * panel->area.paddingy);
	}
	systray.area.parent = p;
	systray.area.panel = p;
}


void cleanup_systray()
{
	systray_enabled = 0;
	systray.area.on_screen = 0;
	free_area(&systray.area);
}


void draw_systray(void *obj, cairo_t *c, int active)
{
	// tint2 don't draw systray icons. just the background.
	refresh_systray = 1;
}


void resize_systray(void *obj)
{
	Systraybar *sysbar = obj;
	Panel *panel = sysbar->area.panel;
	TrayWindow *traywin;
	GSList *l;
	int count, posx, posy;
	int icon_size;

	if (panel_horizontal)
		icon_size = sysbar->area.height;
	else
		icon_size = sysbar->area.width;
	icon_size = icon_size - (2 * sysbar->area.pix.border.width) - (2 * sysbar->area.paddingy);
	count = 0;
	for (l = systray.list_icons; l ; l = l->next) {
		if (!((TrayWindow*)l->data)->hide)
			count++;
	}
	//printf("count %d\n", count);

	if (panel_horizontal) {
		if (!count) systray.area.width = 0;
		else systray.area.width = (2 * systray.area.pix.border.width) + (2 * systray.area.paddingxlr) + (icon_size * count) + ((count-1) * systray.area.paddingx);

		systray.area.posx = panel->area.width - panel->area.pix.border.width - panel->area.paddingxlr - systray.area.width;
		if (panel->clock.area.on_screen)
			systray.area.posx -= (panel->clock.area.width + panel->area.paddingx);
#ifdef ENABLE_BATTERY
		if (panel->battery.area.on_screen)
			systray.area.posx -= (panel->battery.area.width + panel->area.paddingx);
#endif
	}
	else {
		if (!count) systray.area.height = 0;
		else systray.area.height = (2 * systray.area.pix.border.width) + (2 * systray.area.paddingxlr) + (icon_size * count) + ((count-1) * systray.area.paddingx);

		systray.area.posy = panel->area.pix.border.width + panel->area.paddingxlr;
		if (panel->clock.area.on_screen)
			systray.area.posy += (panel->clock.area.height + panel->area.paddingx);
#ifdef ENABLE_BATTERY
		if (panel->battery.area.on_screen)
			systray.area.posy += (panel->battery.area.height + panel->area.paddingx);
#endif
	}

	if (panel_horizontal) {
		posy = panel->area.pix.border.width + panel->area.paddingy + systray.area.pix.border.width + systray.area.paddingy;
		posx = systray.area.posx + systray.area.pix.border.width + systray.area.paddingxlr;
	}
	else {
		posx = panel->area.pix.border.width + panel->area.paddingy + systray.area.pix.border.width + systray.area.paddingy;
		posy = systray.area.posy + systray.area.pix.border.width + systray.area.paddingxlr;
	}
	for (l = systray.list_icons; l ; l = l->next) {
		traywin = (TrayWindow*)l->data;
		if (traywin->hide) continue;

		traywin->y = posy;
		traywin->x = posx;
		traywin->width = icon_size;
		traywin->height = icon_size;
		if (panel_horizontal)
			posx += (icon_size + systray.area.paddingx);
		else
			posy += (icon_size + systray.area.paddingx);

		// position and size the icon window
		XMoveResizeWindow(server.dsp, traywin->id, traywin->x, traywin->y, icon_size, icon_size);
		XResizeWindow(server.dsp, traywin->tray_id, icon_size, icon_size);
	}
}


// ***********************************************
// systray protocol

void start_net()
{
	if (net_sel_win) {
		// protocol already started
		if (!systray_enabled)
			stop_net();
		return;
	}
	else
		if (!systray_enabled)
			return;

	Window win = XGetSelectionOwner(server.dsp, server.atom._NET_SYSTEM_TRAY_SCREEN);

	// freedesktop systray specification
	if (win != None) {
		// search pid
		Atom _NET_WM_PID, actual_type;
		int actual_format;
		unsigned long nitems;
		unsigned long bytes_after;
		unsigned char *prop = 0;
		int pid;

		_NET_WM_PID = XInternAtom(server.dsp, "_NET_WM_PID", True);
		int ret = XGetWindowProperty(server.dsp, win, _NET_WM_PID, 0, 1024, False, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);

		fprintf(stderr, "tint2 : another systray is running");
		if (ret == Success && prop) {
			pid = prop[1] * 256;
			pid += prop[0];
			fprintf(stderr, " pid=%d", pid);
		}
		fprintf(stderr, "\n");
		return;
	}

	// init systray protocol
	net_sel_win = XCreateSimpleWindow(server.dsp, server.root_win, -1, -1, 1, 1, 0, 0, 0);

	// v0.2 trayer specification. tint2 always horizontal.
	// Vertical panel will draw the systray horizontal.
	int orient = 0;
	XChangeProperty(server.dsp, net_sel_win, server.atom._NET_SYSTEM_TRAY_ORIENTATION, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &orient, 1);
	VisualID vid = XVisualIDFromVisual(server.visual);
	XChangeProperty(server.dsp, net_sel_win, XInternAtom(server.dsp, "_NET_SYSTEM_TRAY_VISUAL", False), XA_VISUALID, 32, PropModeReplace, (unsigned char*)&vid, 1);

	XSetSelectionOwner(server.dsp, server.atom._NET_SYSTEM_TRAY_SCREEN, net_sel_win, CurrentTime);
	if (XGetSelectionOwner(server.dsp, server.atom._NET_SYSTEM_TRAY_SCREEN) != net_sel_win) {
		stop_net();
		fprintf(stderr, "tint2 : can't get systray manager\n");
		return;
	}

	//fprintf(stderr, "tint2 : systray started\n");
	XClientMessageEvent ev;
	ev.type = ClientMessage;
	ev.window = server.root_win;
	ev.message_type = server.atom.MANAGER;
	ev.format = 32;
	ev.data.l[0] = CurrentTime;
	ev.data.l[1] = server.atom._NET_SYSTEM_TRAY_SCREEN;
	ev.data.l[2] = net_sel_win;
	ev.data.l[3] = 0;
	ev.data.l[4] = 0;
	XSendEvent(server.dsp, server.root_win, False, StructureNotifyMask, (XEvent*)&ev);
}


void stop_net()
{
	//fprintf(stderr, "tint2 : systray stopped\n");
	if (systray.list_icons) {
		// remove_icon change systray.list_icons
		while(systray.list_icons)
			remove_icon((TrayWindow*)systray.list_icons->data);

		g_slist_free(systray.list_icons);
		systray.list_icons = 0;
	}

	if (net_sel_win != None) {
		XDestroyWindow(server.dsp, net_sel_win);
		net_sel_win = None;
	}
}


gboolean error;
int window_error_handler(Display *d, XErrorEvent *e)
{
	d=d;e=e;
	error = TRUE;
	if (e->error_code != BadWindow) {
		printf("error_handler %d\n", e->error_code);
	}
	return 0;
}


static gint compare_traywindows(gconstpointer a, gconstpointer b)
{
	const TrayWindow * traywin_a = (TrayWindow*)a;
	const TrayWindow * traywin_b = (TrayWindow*)b;
	XTextProperty name_a, name_b;

	if(XGetWMName(server.dsp, traywin_a->tray_id, &name_a) == 0) {
		return -1;
	}
	else if(XGetWMName(server.dsp, traywin_b->tray_id, &name_b) == 0) {
		XFree(name_a.value);
		return 1;
	}
	else {
		gint retval = g_ascii_strncasecmp((char*)name_a.value, (char*)name_b.value, -1) * systray.sort;
		XFree(name_a.value);
		XFree(name_b.value);
		return retval;
	}
}


gboolean add_icon(Window id)
{
	TrayWindow *traywin;
	XErrorHandler old;
	Panel *panel = systray.area.panel;
	int hide = 0;

	error = FALSE;
	int wrong_format = 0;
	old = XSetErrorHandler(window_error_handler);
	XWindowAttributes attr;
	XGetWindowAttributes(server.dsp, id, &attr);
	XSetWindowAttributes set_attr;
	wrong_format = (attr.depth != server.depth);
	set_attr.colormap = attr.colormap;
	set_attr.background_pixel = 0;
	set_attr.border_pixel = 0;
	unsigned long mask = CWColormap|CWBackPixel|CWBorderPixel;
	Window parent_window;
	if (real_transparency)
		parent_window = XCreateWindow(server.dsp, panel->main_win, 0, 0, 30, 30, 0, attr.depth, InputOutput, attr.visual, mask, &set_attr);
	else
		parent_window = panel->main_win;
	XReparentWindow(server.dsp, id, parent_window, 0, 0);
	XSync(server.dsp, False);
	XSetErrorHandler(old);
	if (error != FALSE) {
		fprintf(stderr, "tint2 : not icon_swallow\n");
		return FALSE;
	}

	{
		Atom acttype;
		int actfmt;
		unsigned long nbitem, bytes;
		unsigned char *data = 0;
		int ret;

		ret = XGetWindowProperty(server.dsp, id, server.atom._XEMBED_INFO, 0, 2, False, server.atom._XEMBED_INFO, &acttype, &actfmt, &nbitem, &bytes, &data);
		if (ret == Success) {
			if (data) {
				if (nbitem == 2) {
					//hide = ((data[1] & XEMBED_MAPPED) == 0);
					//printf("hide %d\n", hide);
				}
				XFree(data);
			}
		}
		else {
			fprintf(stderr, "tint2 : xembed error\n");
			return FALSE;
		}
	}
	{
		XEvent e;
		e.xclient.type = ClientMessage;
		e.xclient.serial = 0;
		e.xclient.send_event = True;
		e.xclient.message_type = server.atom._XEMBED;
		e.xclient.window = id;
		e.xclient.format = 32;
		e.xclient.data.l[0] = CurrentTime;
		e.xclient.data.l[1] = XEMBED_EMBEDDED_NOTIFY;
		e.xclient.data.l[2] = 0;
		e.xclient.data.l[3] = parent_window;
		e.xclient.data.l[4] = 0;
		XSendEvent(server.dsp, id, False, 0xFFFFFF, &e);
	}

	traywin = g_new0(TrayWindow, 1);
	if (real_transparency)
		traywin->id = parent_window;
	else
		traywin->id = id;
	traywin->tray_id = id;
	traywin->hide = hide;
	traywin->wrong_format = wrong_format;

	if (systray.sort == 3)
		systray.list_icons = g_slist_prepend(systray.list_icons, traywin);
	else if (systray.sort == 2)
		systray.list_icons = g_slist_append(systray.list_icons, traywin);
	else
		systray.list_icons = g_slist_insert_sorted(systray.list_icons, traywin, compare_traywindows);
	systray.area.resize = 1;
	systray.area.redraw = 1;
	//printf("add_icon id %lx, %d\n", id, g_slist_length(systray.list_icons));

	// watch for the icon trying to resize itself!
	XSelectInput(server.dsp, traywin->tray_id, StructureNotifyMask);
	if (real_transparency) {
		XDamageCreate(server.dsp, traywin->id, XDamageReportRawRectangles);
		XCompositeRedirectWindow(server.dsp, traywin->id, CompositeRedirectManual);
	}

	// show the window
	if (!traywin->hide) {
		XMapRaised(server.dsp, traywin->id);
		XMapRaised(server.dsp, traywin->tray_id);
	}

	// changed in systray force resize on panel
	panel->area.resize = 1;
	panel_refresh = 1;
	return TRUE;
}


void remove_icon(TrayWindow *traywin)
{
	XErrorHandler old;

	// remove from our list
	systray.list_icons = g_slist_remove(systray.list_icons, traywin);
	systray.area.resize = 1;
	systray.area.redraw = 1;
	//printf("remove_icon id %lx, %d\n", traywin->id);

	XSelectInput(server.dsp, traywin->tray_id, NoEventMask);

	// reparent to root
	error = FALSE;
	old = XSetErrorHandler(window_error_handler);
	if (!traywin->hide)
		XUnmapWindow(server.dsp, traywin->id);
	XReparentWindow(server.dsp, traywin->tray_id, server.root_win, 0, 0);
	if (traywin->id != traywin->tray_id)
		XDestroyWindow(server.dsp, traywin->id);
	XSync(server.dsp, False);
	XSetErrorHandler(old);
	g_free(traywin);

	// changed in systray force resize on panel
	Panel *panel = systray.area.panel;
	panel->area.resize = 1;
	panel_refresh = 1;
}


void net_message(XClientMessageEvent *e)
{
	unsigned long opcode;
	Window id;

	opcode = e->data.l[1];
	switch (opcode) {
		case SYSTEM_TRAY_REQUEST_DOCK:
			id = e->data.l[2];
			if (id) add_icon(id);
			break;

		case SYSTEM_TRAY_BEGIN_MESSAGE:
		case SYSTEM_TRAY_CANCEL_MESSAGE:
			// we don't show baloons messages.
			break;

		default:
			if (opcode == server.atom._NET_SYSTEM_TRAY_MESSAGE_DATA)
				printf("message from dockapp: %s\n", e->data.b);
			else
				fprintf(stderr, "SYSTEM_TRAY : unknown message type\n");
			break;
	}
}


void systray_render_icons(TrayWindow* traywin)
{
	// most systray icons support 32 bit depth, but some icons are still 24 bit.
	// We create a heuristic mask for these icons, i.e. we get the rgb value in the top left corner, and
	// mask out all pixel with the same rgb value

	Picture picture_systray, picture_tray, picture_panel;
	Drawable mask, tray_pixmap;
	Panel* panel = systray.area.panel;
	XWindowAttributes attr;
	XGetWindowAttributes(server.dsp, traywin->id, &attr);
	XRenderPictFormat *format = XRenderFindVisualFormat(server.dsp, attr.visual);
	XRenderPictFormat *panel_format = XRenderFindVisualFormat(server.dsp, server.visual);
	if (traywin->wrong_format) {
		imlib_context_set_drawable(traywin->id);
		Imlib_Image image = imlib_create_image_from_drawable(0, 0, 0, traywin->width, traywin->height, 0);
		imlib_context_set_image(image);
		imlib_image_set_has_alpha(1);
		DATA32* data = imlib_image_get_data();
		createHeuristicMask(data, traywin->width, traywin->height);
		imlib_image_put_back_data(data);
		imlib_render_pixmaps_for_whole_image(&tray_pixmap, &mask);
		picture_tray = XRenderCreatePicture( server.dsp, tray_pixmap, panel_format, 0, 0);
		Picture mask2 = XRenderCreatePicture( server.dsp, mask, XRenderFindStandardFormat(server.dsp, PictStandardA1), 0, 0);
		picture_systray = XRenderCreatePicture( server.dsp, systray.area.pix.pmap, panel_format, 0, 0);
		picture_panel = XRenderCreatePicture(server.dsp, panel->main_win, panel_format, 0, 0);
		XRenderComposite(server.dsp, PictOpOver, picture_tray, mask2, picture_systray, 0, 0, 0, 0, traywin->x-systray.area.posx, traywin->y-systray.area.posy, traywin->width, traywin->height);
		XRenderComposite(server.dsp, PictOpOver, picture_tray, mask2, picture_panel, 0, 0, 0, 0, traywin->x, traywin->y, traywin->width, traywin->height);
		imlib_free_pixmap_and_mask(tray_pixmap);
		imlib_free_image();
	}
	else {
		picture_tray = XRenderCreatePicture( server.dsp, traywin->id, format, 0, 0);
		picture_systray = XRenderCreatePicture( server.dsp, systray.area.pix.pmap, panel_format, 0, 0);
		picture_panel = XRenderCreatePicture(server.dsp, panel->main_win, panel_format, 0, 0);
		XRenderComposite(server.dsp, PictOpOver, picture_tray, None, picture_systray, 0, 0, 0, 0, traywin->x-systray.area.posx, traywin->y-systray.area.posy, traywin->width, traywin->height);
		XRenderComposite(server.dsp, PictOpOver, picture_tray, None, picture_panel, 0, 0, 0, 0, traywin->x, traywin->y, traywin->width, traywin->height);
	}
	XRenderFreePicture(server.dsp, picture_systray);
	XRenderFreePicture(server.dsp, picture_tray);
	XRenderFreePicture(server.dsp, picture_panel);
}


void refresh_systray_icon()
{
	TrayWindow *traywin;
	GSList *l;
	for (l = systray.list_icons; l ; l = l->next) {
		traywin = (TrayWindow*)l->data;
		if (traywin->hide) continue;
		if (real_transparency) systray_render_icons(traywin);
		else XClearArea(server.dsp, traywin->id, 0, 0, traywin->width, traywin->height, False);
	}
	if (real_transparency)
		XFlush(server.dsp);
}
