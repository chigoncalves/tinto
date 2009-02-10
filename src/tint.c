/**************************************************************************
*
* Tint2 panel
*
* Copyright (C) 2007 Pål Staurland (staura@gmail.com)
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

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>
#include <Imlib2.h>
#include <signal.h>

#include "server.h"
#include "window.h"
#include "config.h"
#include "task.h"
#include "taskbar.h"
#include "systraybar.h"
#include "panel.h"


void signal_handler(int sig)
{
	// signal handler is light as it should be
	signal_pending = sig;
}


void init ()
{
	// Set signal handler
   signal(SIGUSR1, signal_handler);
   signal(SIGINT, signal_handler);
   signal(SIGTERM, signal_handler);

   // set global data
   memset(&server, 0, sizeof(Server_global));

   server.dsp = XOpenDisplay (NULL);
   if (!server.dsp) {
      fprintf(stderr, "Could not open display.\n");
      exit(0);
   }
   server_init_atoms ();
   server.screen = DefaultScreen (server.dsp);
	server.root_win = RootWindow(server.dsp, server.screen);
   server.depth = DefaultDepth (server.dsp, server.screen);
   server.visual = DefaultVisual (server.dsp, server.screen);
   server.desktop = server_get_current_desktop ();
	XGCValues  gcv;
	server.gc = XCreateGC (server.dsp, server.root_win, (unsigned long)0, &gcv) ;

   XSetErrorHandler ((XErrorHandler) server_catch_error);

   imlib_context_set_display (server.dsp);
   imlib_context_set_visual (server.visual);
   imlib_context_set_colormap (DefaultColormap (server.dsp, server.screen));

   /* Catch events */
   XSelectInput (server.dsp, server.root_win, PropertyChangeMask|StructureNotifyMask);

   setlocale (LC_ALL, "");
}


void cleanup()
{
	cleanup_panel();

   if (time1_font_desc) pango_font_description_free(time1_font_desc);
   if (time2_font_desc) pango_font_description_free(time2_font_desc);
   if (time1_format) g_free(time1_format);
   if (time2_format) g_free(time2_format);

   if (server.monitor) free(server.monitor);
   XFreeGC(server.dsp, server.gc);
   XCloseDisplay(server.dsp);
}


void window_action (Task *tsk, int action)
{
   switch (action) {
      case CLOSE:
         set_close (tsk->win);
         break;
      case TOGGLE:
         set_active(tsk->win);
         break;
      case ICONIFY:
         XIconifyWindow (server.dsp, tsk->win, server.screen);
         break;
      case TOGGLE_ICONIFY:
         if (tsk == task_active) XIconifyWindow (server.dsp, tsk->win, server.screen);
         else set_active (tsk->win);
         break;
      case SHADE:
         window_toggle_shade (tsk->win);
         break;
   }
}


void event_button_press (XEvent *e)
{
   Panel *panel = get_panel(e->xany.window);
	if (!panel) return;

   if (panel_mode != MULTI_DESKTOP) {
      // drag and drop disabled
      XLowerWindow (server.dsp, panel->main_win);
      return;
   }

   GSList *l0;
   Taskbar *tskbar;
   int x = e->xbutton.x;
   int y = e->xbutton.y;
   for (l0 = panel->area.list; l0 ; l0 = l0->next) {
      tskbar = l0->data;
      if (x >= tskbar->area.posx && x <= (tskbar->area.posx + tskbar->area.width))
         break;
   }

   if (l0) {
      Task *tsk;
      for (l0 = tskbar->area.list; l0 ; l0 = l0->next) {
         tsk = l0->data;
         if (x >= tsk->area.posx && x <= (tsk->area.posx + tsk->area.width)) {
            task_drag = tsk;
            break;
         }
      }
   }

   XLowerWindow (server.dsp, panel->main_win);
}


void event_button_release (XEvent *e)
{
   // TODO: convert event_button_press(int x, int y) to area->event_button_press()

   Panel *panel = get_panel(e->xany.window);
	if (!panel) return;

   int action = TOGGLE_ICONIFY;
   int x = e->xbutton.x;
   int y = e->xbutton.y;
   switch (e->xbutton.button) {
      case 2:
         action = mouse_middle;
         break;
      case 3:
         action = mouse_right;
         break;
      case 4:
         action = mouse_scroll_up;
         break;
      case 5:
         action = mouse_scroll_down;
         break;
   }

   // search taskbar
   Taskbar *tskbar;
   GSList *l0;
   for (l0 = panel->area.list; l0 ; l0 = l0->next) {
      tskbar = l0->data;
      if (x >= tskbar->area.posx && x <= (tskbar->area.posx + tskbar->area.width))
         goto suite;
   }

   // TODO: check better solution to keep window below
   XLowerWindow (server.dsp, panel->main_win);
   task_drag = 0;
   return;

suite:
   // drag and drop task
   if (task_drag) {
      if (tskbar != task_drag->area.parent && action == TOGGLE_ICONIFY) {
         if (task_drag->desktop != ALLDESKTOP && panel_mode == MULTI_DESKTOP) {
            windows_set_desktop(task_drag->win, tskbar->desktop);
            if (tskbar->desktop == server.desktop)
               set_active(task_drag->win);
            task_drag = 0;
         }
         return;
      }
      else task_drag = 0;
   }

   // switch desktop
   if (panel_mode == MULTI_DESKTOP)
      if (tskbar->desktop != server.desktop && action != CLOSE)
         set_desktop (tskbar->desktop);

   // action on task
   Task *tsk;
   GSList *l;
   for (l = tskbar->area.list ; l ; l = l->next) {
      tsk = l->data;
      if (x >= tsk->area.posx && x <= (tsk->area.posx + tsk->area.width)) {
         window_action (tsk, action);
         break;
      }
   }

   // to keep window below
   XLowerWindow (server.dsp, panel->main_win);
}


void event_property_notify (Window win, Atom at)
{
  	int i, j;
   Task *tsk;

   if (win == server.root_win) {
      if (!server.got_root_win) {
         XSelectInput (server.dsp, server.root_win, PropertyChangeMask|StructureNotifyMask);
         server.got_root_win = 1;
      }

      /* Change number of desktops */
      else if (at == server.atom._NET_NUMBER_OF_DESKTOPS) {
	      server.nb_desktop = server_get_number_of_desktop ();
		  	cleanup_taskbar();
			init_taskbar();
			visible_object();
			for (i=0 ; i < nb_panel ; i++) {
				set_resize(&panel1[i]);
			}
			task_refresh_tasklist();
			panel_refresh = 1;
      }
      /* Change desktop */
      else if (at == server.atom._NET_CURRENT_DESKTOP) {
         server.desktop = server_get_current_desktop ();
         if (panel_mode != MULTI_DESKTOP) {
				visible_object();
         }
      }
      /* Window list */
      else if (at == server.atom._NET_CLIENT_LIST) {
         task_refresh_tasklist();
         panel_refresh = 1;
      }
      /* Change active */
      else if (at == server.atom._NET_ACTIVE_WINDOW) {
         GSList *l0;
      	if (task_active) {
				for (i=0 ; i < nb_panel ; i++) {
					for (j=0 ; j < panel1[i].nb_desktop ; j++) {
                  for (l0 = panel1[i].taskbar[j].area.list; l0 ; l0 = l0->next) {
                     tsk = l0->data;
             		   tsk->area.is_active = 0;
                  }
					}
				}
         	task_active = 0;
			}
         Window w1 = window_get_active ();
         Task *t = task_get_task(w1);
         if (!t) {
            Window w2;
            if (XGetTransientForHint(server.dsp, w1, &w2) != 0)
               if (w2) t = task_get_task(w2);
         }
         if (t) {
				for (i=0 ; i < nb_panel ; i++) {
					for (j=0 ; j < panel1[i].nb_desktop ; j++) {
                  for (l0 = panel1[i].taskbar[j].area.list; l0 ; l0 = l0->next) {
                     tsk = l0->data;
                     if (tsk->win == t->win) {
                  		tsk->area.is_active = 1;
                  		//printf("active monitor %d, task %s\n", panel1[i].monitor, tsk->title);
                  	}
                  }
					}
				}
         	task_active = t;
			}
         panel_refresh = 1;
      }
      /* Wallpaper changed */
      else if (at == server.atom._XROOTPMAP_ID) {
			for (i=0 ; i < nb_panel ; i++) {
				set_panel_background(&panel1[i]);
			}
         panel_refresh = 1;
      }
   }
   else {
      tsk = task_get_task (win);
      if (!tsk) return;
      //printf("atom root_win = %s, %s\n", XGetAtomName(server.dsp, at), tsk->title);

      /* Window title changed */
      if (at == server.atom._NET_WM_VISIBLE_NAME || at == server.atom._NET_WM_NAME || at == server.atom.WM_NAME) {
			Task *tsk2;
			GSList *l0;
			get_title(tsk);
			// changed other tsk->title
			for (i=0 ; i < nb_panel ; i++) {
				for (j=0 ; j < panel1[i].nb_desktop ; j++) {
					for (l0 = panel1[i].taskbar[j].area.list; l0 ; l0 = l0->next) {
						tsk2 = l0->data;
						if (tsk->win == tsk2->win && tsk != tsk2) {
							tsk2->title = tsk->title;
							tsk2->area.redraw = 1;
						}
					}
				}
			}
         panel_refresh = 1;
      }
      /* Iconic state */
      else if (at == server.atom.WM_STATE) {
         if (window_is_iconified (win))
            if (task_active) {
               if (task_active->win == tsk->win) {
						Task *tsk2;
						GSList *l0;
						for (i=0 ; i < nb_panel ; i++) {
							for (j=0 ; j < panel1[i].nb_desktop ; j++) {
								for (l0 = panel1[i].taskbar[j].area.list; l0 ; l0 = l0->next) {
									tsk2 = l0->data;
									tsk2->area.is_active = 0;
								}
							}
						}
                  task_active = 0;
               }
            }
      }
      /* Window icon changed */
      else if (at == server.atom._NET_WM_ICON) {
		   get_icon(tsk);
			Task *tsk2;
			GSList *l0;
			for (i=0 ; i < nb_panel ; i++) {
				for (j=0 ; j < panel1[i].nb_desktop ; j++) {
					for (l0 = panel1[i].taskbar[j].area.list; l0 ; l0 = l0->next) {
						tsk2 = l0->data;
						if (tsk->win == tsk2->win && tsk != tsk2) {
							tsk2->icon_width = tsk->icon_width;
							tsk2->icon_height = tsk->icon_height;
							tsk2->icon_data = tsk->icon_data;
							tsk2->area.redraw = 1;
						}
					}
				}
			}
         panel_refresh = 1;
      }
      /* Window desktop changed */
      else if (at == server.atom._NET_WM_DESKTOP) {
         remove_task (tsk);
         add_task (win);
         panel_refresh = 1;
      }

      if (!server.got_root_win) server.root_win = RootWindow (server.dsp, server.screen);
   }
}


void event_configure_notify (Window win)
{
   if (panel_mode != SINGLE_MONITOR) return;
   if (server.nb_monitor == 1) return;

   Task *tsk = task_get_task (win);
   if (!tsk) return;

   Panel *p = tsk->area.panel;
   if (p->monitor != window_get_monitor (win)) {
      // task on another monitor
      remove_task (tsk);
      add_task (win);
      panel_refresh = 1;
   }
}


void event_timer()
{
   struct timeval stv;

   if (!time1_format) return;

   if (gettimeofday(&stv, 0)) return;

   if (abs(stv.tv_sec - time_clock.tv_sec) < time_precision) return;

   // update clock
   time_clock.tv_sec = stv.tv_sec;
   time_clock.tv_sec -= time_clock.tv_sec % time_precision;

	int i;
	for (i=0 ; i < nb_panel ; i++) {
	   panel1[i].clock.area.redraw = 1;
	   panel1[i].clock.area.resize = 1;
	}
   panel_refresh = 1;
}


int main (int argc, char *argv[])
{
   XEvent e;
   fd_set fd;
   int x11_fd, i, c;
   struct timeval tv;
   Panel *panel;

   c = getopt (argc, argv, "c:");
   init ();

load_config:
   i = 0;
	init_config();
   if (c != -1)
      i = config_read_file (optarg);
   if (!i)
      i = config_read ();
   if (!i) {
      fprintf(stderr, "usage: tint2 [-c] <config_file>\n");
      cleanup();
      exit(1);
   }
   config_finish();

   x11_fd = ConnectionNumber(server.dsp);
   XSync(server.dsp, False);

   while (1) {
      // thanks to AngryLlama for the timer
      // Create a File Description Set containing x11_fd
      FD_ZERO (&fd);
      FD_SET (x11_fd, &fd);

      tv.tv_usec = 500000;
      tv.tv_sec = 0;

      // Wait for X Event or a Timer
      if (select(x11_fd+1, &fd, 0, 0, &tv)) {
         while (XPending (server.dsp)) {
            XNextEvent(server.dsp, &e);

            switch (e.type) {
               case ButtonPress:
                  //printf("ButtonPress %lx\n", e.xproperty.window);
                  if (e.xbutton.button == 1) event_button_press (&e);
                  break;

               case ButtonRelease:
                  event_button_release (&e);
                  break;

               case Expose:
               	panel = get_panel(e.xany.window);
               	if (!panel) break;
                  //XCopyArea (server.dsp, panel.area.pix.pmap, server.root_win, server.gc_root, 0, 0, panel.area.width, panel.area.height, server.posx, server.posy);
                  //XCopyArea (server.dsp, server.pmap, panel.main_win, server.gc, panel.area.paddingxlr, 0, panel.area.width-(2*panel.area.paddingxlr), panel.area.height, 0, 0);
                  XCopyArea (server.dsp, panel->root_pmap, panel->main_win, server.gc, 0, 0, panel->area.width, panel->area.height, 0, 0);
                  break;

               case PropertyNotify:
                  event_property_notify (e.xproperty.window, e.xproperty.atom);
                  break;

               case ConfigureNotify:
                  if (e.xconfigure.window == server.root_win)
                     goto load_config;
                  else
                     event_configure_notify (e.xconfigure.window);
                  break;

					case UnmapNotify:
					case DestroyNotify:
					/*
						GSList *it;
						for (it = icons; it; it = g_slist_next(it)) {
							if (((TrayWindow*)it->data)->id == e.xany.window) {
								icon_remove(it);
								break;
							}
						}*/
					break;

					case ClientMessage:
						break;
						if (e.xclient.message_type == server.atom._NET_SYSTEM_TRAY_OPCODE && e.xclient.format == 32)
						// &&	e.xclient.window == net_sel_win)
							net_message(&e.xclient);
                  break;
            }
         }
      }
      else event_timer();

		switch (signal_pending) {
			case SIGUSR1:
            goto load_config;
			case SIGINT:
			case SIGTERM:
			   cleanup ();
			   return 0;
      }

      if (panel_refresh) {
			for (i=0 ; i < nb_panel ; i++) {
	         visual_refresh(&panel1[i]);
			}

			XFlush (server.dsp);
			panel_refresh = 0;
		}
   }
}


