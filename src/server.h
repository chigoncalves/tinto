/**************************************************************************
* server : 
* - 
*
* Check COPYING file for Copyright
*
**************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xinerama.h>


typedef struct Global_atom
{
        Atom _XROOTPMAP_ID;
        Atom _NET_CURRENT_DESKTOP;
        Atom _NET_NUMBER_OF_DESKTOPS;
        Atom _NET_DESKTOP_GEOMETRY;
        Atom _NET_DESKTOP_VIEWPORT;
        Atom _NET_ACTIVE_WINDOW;
        Atom _NET_WM_WINDOW_TYPE;
        Atom _NET_WM_STATE_SKIP_PAGER;
        Atom _NET_WM_STATE_SKIP_TASKBAR;
        Atom _NET_WM_STATE_STICKY;
        Atom _NET_WM_WINDOW_TYPE_DOCK;
        Atom _NET_WM_WINDOW_TYPE_DESKTOP;
        Atom _NET_WM_WINDOW_TYPE_TOOLBAR;
        Atom _NET_WM_WINDOW_TYPE_MENU;
        Atom _NET_WM_WINDOW_TYPE_SPLASH;
        Atom _NET_WM_WINDOW_TYPE_DIALOG;
        Atom _NET_WM_WINDOW_TYPE_NORMAL;
        Atom _NET_WM_DESKTOP;
        Atom WM_STATE;
        Atom _NET_WM_STATE;
        Atom _NET_WM_STATE_SHADED;
        Atom _NET_WM_STATE_BELOW;
        Atom _NET_WM_STATE_MODAL;
        Atom _NET_CLIENT_LIST;
        Atom _NET_WM_NAME;
        Atom _NET_WM_VISIBLE_NAME;
        Atom _NET_WM_STRUT;
        Atom _NET_WM_ICON;
        Atom _NET_CLOSE_WINDOW;
        Atom UTF8_STRING;
        Atom _NET_SUPPORTING_WM_CHECK;
        Atom _WIN_LAYER;
        Atom _NET_WM_STRUT_PARTIAL;
        Atom WM_NAME;
        Atom __SWM_VROOT;
        Atom _MOTIF_WM_HINTS;
} Global_atom;



typedef struct Monitor
{
        int x;
        int y;
        int width;
        int height;
} Monitor;


typedef struct
{
        Display *dsp;
        Window root_win;
        int desktop;
        int screen;
        int depth;
        int nb_desktop;
        Monitor *monitor;
        int nb_monitor;
        int got_root_win;
        Visual *visual;
        int posx, posy;
        Pixmap pmap;
        GC gc;
        GC gc_root;
        Global_atom atom;
} Server_global;


Server_global server;


void send_event32 (Window win, Atom at, long data1, long data2);
int  get_property32 (Window win, Atom at, Atom type);
void *server_get_property (Window win, Atom at, Atom type, int *num_results);
Atom server_get_atom (char *atom_name);
void server_refresh_root_pixmap ();
void server_refresh_main_pixmap ();
void server_catch_error (Display *d, XErrorEvent *ev);
void server_init_atoms ();
void get_monitors();
Pixmap get_root_pixmap();


#endif
