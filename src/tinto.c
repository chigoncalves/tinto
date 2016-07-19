#include "conf.h"


#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>
#include <X11/extensions/Xdamage.h>
#include <Imlib2.h>
#include <signal.h>


#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_BATTERY
  #include "battery.h"
#endif // ENABLE_BATTERY
#include "clock.h"
#include "config.h"
#include "debug.h"
#include "launcher.h"
#include "taskbar.h"
#include "timer.h"
#include "tinto.h"
#include "tooltip.h"
#include "server.h"

#include "window.h"
#include "config.h"
#include "task.h"
#include "systraybar.h"
#include "xsettings-client.h"


Window dnd_source_window;
Window dnd_target_window;
int dnd_version;
Atom dnd_selection;
Atom dnd_atom;
int dnd_sent_request;
char *dnd_launcher_exec;


int pending_signal;

static void
tinto_init_x11 (void);

noreturn void
tinto_usage (void) {
  MSG ("Usage\n");
  MSG ("  %s [options]\n", PROJECT_NAME);

  MSG ("Options");
  MSG ("  --config-file, -c <filename>%63s", "Start tinto using"
       " \"filename\" as config file.\n");
  MSG ("  --help, -h%71s", "Print this help message then exit.\n");
  MSG ("  --panel-snapshot, -s%86s", "<path-to-a-new-image>    Take a new"
       " snapshot of the panel an save it as \"new-image\".\n");
  MSG ("  --version, -v%69s", "Print version information then exit.");

  exit (EXIT_FAILURE);
}

void
tinto_signal_handler(int sig) {
  pending_signal = sig;
}

void
tinto_init (int argc, char *argv[]) {
  default_config ();
  default_timeout ();
  default_systray();
  memset (&server, 0, sizeof (Server_global));
#ifdef ENABLE_BATTERY
  default_battery ();
#endif // ENABLE_BATTERY
  default_clock ();
  default_launcher ();
  default_taskbar ();
  default_tooltip ();
  panel_default ();

  for (int i = 1; i < argc; ++i) {
    if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
      tinto_usage ();
    }

    else if (strcmp (argv[i], "-v") == 0 ||
	     strcmp (argv[i], "--version") == 0) {
      MSG ("%s version %s\n", PROJECT_NAME, PROJECT_VERSION);
      exit (EXIT_SUCCESS);
    }
    else if (strcmp (argv[i], "-c") == 0
	     || strcmp (argv[i], "--config-file") == 0) {
      if (++i < argc) config_path = strdup (argv[i]);
    }
    else if (strcmp (argv[i], "-s") == 0
	     || strcmp (argv[i], "--panel-snapshot") == 0) {
      if (++i < argc) snapshot_path = strdup (argv[i]);
    }
    else {
      MSG ("Invalid argument!");
      tinto_usage ();
    }
  }

  // Isn't this initialization redundant? Since `pending_signal'
  // is static and all?
  pending_signal = 0;
  struct sigaction sa = { .sa_handler = tinto_signal_handler };
  struct sigaction sa_chld = {
    .sa_handler = SIG_DFL,
    .sa_flags = SA_NOCLDWAIT
  };
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGCHLD, &sa_chld, NULL);

  // BSD does not support pselect(), therefore we have to use select and hope that we do not
  // end up in a race condition there (see 'man select()' on a linux machine for more information)
  // block all signals, such that no race conditions occur before pselect in our main loop
  //	sigset_t block_mask;
  //	sigaddset(&block_mask, SIGINT);
  //	sigaddset(&block_mask, SIGTERM);
  //	sigaddset(&block_mask, SIGHUP);
  //	sigaddset(&block_mask, SIGUSR1);
  //	sigprocmask(SIG_BLOCK, &block_mask, 0);

  tinto_init_x11 ();
}

void
tinto_init_x11 (void) {
  server.dsp = XOpenDisplay (getenv ("DISPLAY"));
  if (!server.dsp)
    DIE ("%s Failed to open display.", PROJECT_NAME);

  server_init_atoms ();
  server.screen = DefaultScreen (server.dsp);
  server.root_win = RootWindow (server.dsp, server.screen);
  server.desktop = server_get_current_desktop ();

  // config file use '.' as decimal separator
  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "POSIX");

  // get monitor and desktop config
  get_monitors ();
  get_desktops ();

  server.disable_transparency = 0;

  server_init_visual ();

  imlib_context_set_display (server.dsp);
  imlib_context_set_visual (server.visual);
  imlib_context_set_colormap (server.colormap);

  /* Catch events */
  XSelectInput (server.dsp, server.root_win,
		PropertyChangeMask | StructureNotifyMask);

  gchar *path;
  const gchar * const *data_dirs = g_get_system_data_dirs ();
  for (size_t i = 0; data_dirs[i] != NULL; ++i)	{
    path = g_build_filename (data_dirs[i], "tint2", "icon.png", NULL);
    if (g_file_test (path, G_FILE_TEST_EXISTS))
      default_icon = imlib_load_image (path);

    g_free (path);
  }
}

void
tinto_deinit (void) {
  cleanup_systray ();
  cleanup_tooltip ();
  cleanup_clock ();
  launcher_deinit ();

#ifdef ENABLE_BATTERY
  cleanup_battery ();
#endif
  panel_cleanup ();
  cleanup_config ();

  if (default_icon) {
    imlib_context_set_image (default_icon);
    imlib_free_image ();
    default_icon = NULL;
  }
  imlib_context_disconnect_display ();

  cleanup_server ();
  cleanup_timeout ();
  if (server.dsp)
    XCloseDisplay (server.dsp);

  server.dsp = NULL;
}

void
tinto_take_snapshot(const char *path) {
  Panel* panel = &panel1[0];

  if (panel->area.bounds.width > server.monitor[0].width)
    panel->area.bounds.width = server.monitor[0].width;

  panel->temp_pmap = XCreatePixmap (server.dsp, server.root_win,
				    panel->area.bounds.width,
				    panel->area.bounds.height,
				    server.depth);
  rendering (panel);

  imlib_context_set_drawable (panel->temp_pmap);
  Imlib_Image img =
    imlib_create_image_from_drawable (None, 0, 0,
				      panel->area.bounds.width,
				      panel->area.bounds.height, 0);

  imlib_context_set_image (img);
  if (!panel_horizontal) {
    imlib_image_flip_horizontal ();
    imlib_image_flip_diagonal ();
  }
  imlib_save_image (path);
  imlib_free_image ();
}

int
tint2_handles_click(Panel* panel, XButtonEvent* e) {
  point_t point = { .x = e->x, .y = e-> y };
  Task* task = panel_click_task (panel, point);
  if (task) {
    if(   (e->button == 1 && mouse_left != 0)
	  || (e->button == 2 && mouse_middle != 0)
	  || (e->button == 3 && mouse_right != 0)
	  || (e->button == 4 && mouse_scroll_up != 0)
	  || (e->button == 5 && mouse_scroll_down !=0) ) {
      return 1;
    }
    else
      return 0;
  }

  LauncherIcon *icon = panel_click_launcher_icon (panel, point);
  if (icon) {
    if (e->button == 1) {
      return 1;
    }
    else {
      return 0;
    }
  }

  // no launcher/task clicked --> check if taskbar clicked
  Taskbar *tskbar = panel_click_taskbar (panel, point);
  if (tskbar && e->button == 1 && panel_mode == MULTI_DESKTOP)
    return 1;

  if (panel_click_clock (panel, point)) {
    if ((e->button == 1 && clock_lclick_command)
	|| (e->button == 3 && clock_rclick_command) )
      return 1;
    else
      return 0;
  }
  return 0;
}

void
forward_click (XEvent* e) {
  // forward the click to the desktop window (thanks conky)
  XUngrabPointer (server.dsp, e->xbutton.time);
  e->xbutton.window = server.root_win;
  // icewm doesn't open under the mouse.
  // and xfce doesn't open at all.
  e->xbutton.x = e->xbutton.x_root;
  e->xbutton.y = e->xbutton.y_root;
  //printf("**** %d, %d\n", e->xbutton.x, e->xbutton.y);
  //XSetInputFocus(server.dsp, e->xbutton.window, RevertToParent, e->xbutton.time);
  XSendEvent (server.dsp, e->xbutton.window, False,
	      ButtonPressMask, e);
}

void
event_button_press (XEvent *e) {
  Panel *panel = panel_get (e->xany.window);
  if (!panel) return;

  if (wm_menu && !tint2_handles_click (panel, &e->xbutton) ) {
    forward_click (e);
    return;
  }

  task_drag = panel_click_task (panel,
				(point_t){e->xbutton.x, e->xbutton.y});

  if (panel_layer == BOTTOM_LAYER)
    XLowerWindow (server.dsp, panel->main_win);
}


void
event_button_motion_notify (XEvent *e) {
  Panel * panel = panel_get (e->xany.window);
  if(!panel || !task_drag)
    return;

  // Find the taskbar on the event's location
  point_t point = {e->xbutton.x, e->xbutton.y};
  Taskbar * event_taskbar = panel_click_taskbar(panel, point);
  if(!event_taskbar)
    return;

  // Find the task on the event's location
  Task * event_task = panel_click_task (panel, point);

  // If the event takes place on the same taskbar as the task being dragged
  if(event_taskbar == task_drag->area.parent)	{
    if (taskbar_sort_method != TASKBAR_NOSORT) {
      sort_tasks (event_taskbar);
    }
    else {
      // Swap the task_drag with the task on the event's location (if they differ)
      if (event_task && event_task != task_drag) {
	GSList* drag_iter = g_slist_find (event_taskbar->area.list,
					   task_drag);
	GSList* task_iter = g_slist_find (event_taskbar->area.list,
					  event_task);
	if (drag_iter && task_iter) {
	  gpointer temp = task_iter->data;
	  task_iter->data = drag_iter->data;
	  drag_iter->data = temp;
	  event_taskbar->area.resize = 1;
	  panel_refresh = 1;
	  task_dragged = 1;
	}
      }
    }
  }
  else {
    // The event is on another taskbar than the task being dragged
    if (task_drag->desktop == ALLDESKTOP || panel_mode != MULTI_DESKTOP)
      return;

    Taskbar* drag_taskbar = (Taskbar*)task_drag->area.parent;
    drag_taskbar->area.list = g_slist_remove (drag_taskbar->area.list,
					      task_drag);

    if (event_taskbar->area.bounds.x > drag_taskbar->area.bounds.x
       || event_taskbar->area.bounds.y > drag_taskbar->area.bounds.y) {
      int i = (taskbarname_enabled) ? 1 : 0;
      event_taskbar->area.list = g_slist_insert (event_taskbar->area.list,
						 task_drag, i);
    }
    else
      event_taskbar->area.list = g_slist_append (event_taskbar->area.list,
						 task_drag);

    // Move task to other desktop (but avoid the 'Window desktop
    // changed' code in 'event_property_notify')
    task_drag->area.parent = event_taskbar;
    task_drag->desktop = event_taskbar->desktop;

    windows_set_desktop (task_drag->win, event_taskbar->desktop);

    if (taskbar_sort_method != TASKBAR_NOSORT)
      sort_tasks(event_taskbar);

    event_taskbar->area.resize = 1;
    drag_taskbar->area.resize = 1;
    task_dragged = 1;
    panel_refresh = 1;
    panel->area.resize = 1;
  }
}

void
event_button_release (XEvent *e) {
  Panel *panel = panel_get (e->xany.window);
  if (!panel) return;

  if (wm_menu && !tint2_handles_click(panel, &e->xbutton)) {
    forward_click (e);
    if (panel_layer == BOTTOM_LAYER)
      XLowerWindow (server.dsp, panel->main_win);
    task_drag = 0;
    return;
  }

  int action = TOGGLE_ICONIFY;
  switch (e->xbutton.button) {
  case 1:
    action = mouse_left;
    break;

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

  case 6:
    action = mouse_tilt_left;
    break;

  case 7:
    action = mouse_tilt_right;
    break;

  default:
    break;
  }

  point_t btn_location = { e->xbutton.x, e->xbutton.y};
  if (panel_click_clock (panel, btn_location)) {
    clock_action (e->xbutton.button);
    if (panel_layer == BOTTOM_LAYER)
      XLowerWindow (server.dsp, panel->main_win);
    task_drag = 0;
    return;
  }

  if (e->xbutton.button  && panel_click_launcher (panel,
						  btn_location)) {
    LauncherIcon *icon = panel_click_launcher_icon (panel,
						    btn_location);
    if (icon) launcher_action(icon, e);

    task_drag = 0;
    return;
  }

  Taskbar *tskbar = panel_click_taskbar (panel, btn_location);
  if (!tskbar) {
    // TODO: check better solution to keep window below
    if (panel_layer == BOTTOM_LAYER)
      XLowerWindow (server.dsp, panel->main_win);
    task_drag = 0;
    return;
  }

  // drag and drop task
  if (task_dragged) {
    task_drag = 0;
    task_dragged = 0;
    return;
  }

  // switch desktop
  if (panel_mode == MULTI_DESKTOP) {
    if (tskbar->desktop != server.desktop && action != CLOSE
	&& action != DESKTOP_LEFT && action != DESKTOP_RIGHT)
      set_desktop (tskbar->desktop);
  }

  // action on task
  window_action (panel_click_task (panel, btn_location), action);

  // to keep window below
  if (panel_layer == BOTTOM_LAYER)
    XLowerWindow (server.dsp, panel->main_win);
}

void
event_property_notify (XEvent *e) {

  Task *tsk;
  Window win = e->xproperty.window;
  Atom at = e->xproperty.atom;

  if (xsettings_client)
    xsettings_client_process_event (xsettings_client, e);
  if (win == server.root_win) {
    if (!server.got_root_win) {
      XSelectInput (server.dsp, server.root_win,
		    PropertyChangeMask | StructureNotifyMask);
      server.got_root_win = 1;
    }

    // Change name of desktops
    else if (at == server.atom._NET_DESKTOP_NAMES) {
      if (!taskbarname_enabled) return;

      GSList* l = NULL;
      GSList* list = server_get_name_of_desktop ();
      gchar* name = NULL;
      Taskbar* tskbar = NULL;
      for (int i = 0 ; i < nb_panel ; ++i) {
	l = list;
	for (int j = 0; j < panel1[i].nb_desktop; j++) {
	  if (l) {
	    name = g_strdup (l->data);
	    l = l->next;
	  }
	  else
	    name = g_strdup_printf ("%d", j + 1);
	  tskbar = &panel1[i].taskbar[j];
	  if (strcmp (name, tskbar->bar_name.name) != 0) {
	    g_free (tskbar->bar_name.name);
	    tskbar->bar_name.name = name;
	    tskbar->bar_name.area.resize = 1;
	  }
	  else
	    g_free (name);
	}
      }
      l = list;
      while (l) {
	g_free (l->data);
        l = l->next;
      }

      g_slist_free (list);
      panel_refresh = 1;
    }
    // Change number of desktops
    else if (at == server.atom._NET_NUMBER_OF_DESKTOPS) {
      if (!taskbar_enabled) return;

      server.nb_desktop = server_get_number_of_desktops ();
      if (server.nb_desktop <= server.desktop) {
	server.desktop = server.nb_desktop - 1;
      }

      cleanup_taskbar ();
      init_taskbar ();
      for (int i = 0 ; i < nb_panel ; ++i) {
	init_taskbar_panel (&panel1[i]);
	panel_set_items_order (&panel1[i]);
	visible_taskbar (&panel1[i]);
	panel1[i].area.resize = 1;
      }
      task_refresh_tasklist ();
      active_task ();
      panel_refresh = 1;
    }
    // Change desktop
    else if (at == server.atom._NET_CURRENT_DESKTOP) {
      if (!taskbar_enabled) return;
      int old_desktop = server.desktop;
      server.desktop = server_get_current_desktop ();
      for (int i=0 ; i < nb_panel ; ++i) {
	Panel* panel = &panel1[i];
	set_taskbar_state (&panel->taskbar[old_desktop],
			   TASKBAR_NORMAL);
	set_taskbar_state(&panel->taskbar[server.desktop],
			  TASKBAR_ACTIVE);
	// check ALLDESKTOP task => resize taskbar
	Taskbar* tskbar = NULL;
	Task* tsk = NULL;
	GSList* l = NULL;
	if (server.nb_desktop > old_desktop) {
	  tskbar = &panel->taskbar[old_desktop];
	  l = tskbar->area.list;
	  if (taskbarname_enabled) l = l->next;
	  while (l) {
	    tsk = l->data;
	    if (tsk->desktop == ALLDESKTOP) {
	      tsk->area.visible = 0;
	      tskbar->area.resize = 1;
	      panel_refresh = 1;
	      if (panel_mode == MULTI_DESKTOP)
		panel->area.resize = 1;
	    }
	    l = l->next;
	  }
	}

	tskbar = &panel->taskbar[server.desktop];
	l = tskbar->area.list;
	if (taskbarname_enabled) l = l->next;
	while (l) {
	  tsk = l->data;
	  if (tsk->desktop == ALLDESKTOP) {
	    tsk->area.visible = 1;
	    tskbar->area.resize = 1;
	    if (panel_mode == MULTI_DESKTOP)
	      panel->area.resize = 1;
	  }
	  l = l->next;
	}
      }
    }
    // Window list
    else if (at == server.atom._NET_CLIENT_LIST) {
      task_refresh_tasklist ();
      panel_refresh = 1;
    }
    // Change active
    else if (at == server.atom._NET_ACTIVE_WINDOW) {
      active_task ();
      panel_refresh = 1;
    }
    else if (at == server.atom._XROOTPMAP_ID ||
	     at == server.atom._XROOTMAP_ID) {
      // change Wallpaper
      for (int i=0 ; i < nb_panel ; ++i) {
	panel_set_background (&panel1[i]);
      }
      panel_refresh = 1;
    }
  }
  else {
    tsk = task_get_task (win);
    if (!tsk) {
      if (at != server.atom._NET_WM_STATE) return;
      else {
	// xfce4 sends _NET_WM_STATE after minimized to tray, so we
	// need to check if window is mapped if it is mapped and not
	// set as skip_taskbar, we must add it to our task list.
	XWindowAttributes wa;
	XGetWindowAttributes (server.dsp, win, &wa);
	if (wa.map_state == IsViewable
	    && !window_is_skip_taskbar (win)) {
	  if ((tsk = add_task(win)))
	    panel_refresh = 1;
	  else return;
	}
	else return;
      }
    }

    // Window title changed
    if (at == server.atom._NET_WM_VISIBLE_NAME ||
	at == server.atom._NET_WM_NAME || at == server.atom.WM_NAME) {
      if (get_title (tsk)) {
	if (g_tooltip.mapped && (g_tooltip.area == (Area*)tsk)) {
	  tooltip_copy_text ((Area*)tsk);
	  tooltip_update ();
	}
	if (taskbar_sort_method == TASKBAR_SORT_TITLE)
	  sort_taskbar_for_win (win);
	panel_refresh = 1;
      }
    }
    // Demand attention
    else if (at == server.atom._NET_WM_STATE) {
      if (window_is_urgent (win))
	add_urgent(tsk);

      if (window_is_skip_taskbar (win)) {
	remove_task (tsk);
	panel_refresh = 1;
      }
    }
    else if (at == server.atom.WM_STATE) {
      // Iconic state
      int state = (task_active && tsk->win == task_active->win ?
		   TASK_ACTIVE : TASK_NORMAL);
      if (window_is_iconified (win))
	state = TASK_ICONIFIED;

      set_task_state (tsk, state);
      panel_refresh = 1;
    }
    // Window icon changed
    else if (at == server.atom._NET_WM_ICON) {
      get_icon (tsk);
      panel_refresh = 1;
    }
    // Window desktop changed
    else if (at == server.atom._NET_WM_DESKTOP) {
      int desktop = window_get_desktop (win);
      // bug in windowmaker : send unecessary 'desktop changed' when
      // focus changed
      if (desktop != (int)tsk->desktop) {
	remove_task (tsk);
	tsk = add_task (win);
	active_task ();
	panel_refresh = 1;
      }
    }
    else if (at == server.atom.WM_HINTS) {
      XWMHints* wmhints = XGetWMHints (server.dsp, win);
      if (wmhints && wmhints->flags & XUrgencyHint) {
	add_urgent (tsk);
      }
      XFree (wmhints);
    }

    if (!server.got_root_win)
      server.root_win = RootWindow (server.dsp, server.screen);
  }
}

void
event_expose (XEvent *e) {
  Panel* panel = panel_get (e->xany.window);
  if (!panel) return;

  // TODO : one panel_refresh per panel ?
  panel_refresh = 1;
}

void
event_configure_notify (Window win) {
  // change in root window (xrandr)
  if (win == server.root_win) {
    pending_signal = SIGUSR1;
    return;
  }

  // 'win' is a trayer icon
  TrayWindow *traywin;
  GSList *l;
  for (l = systray.list_icons; l ; l = l->next) {
    traywin = (TrayWindow*)l->data;
    if (traywin->tray_id == win) {
      //printf("move tray %d\n", traywin->x);
      XMoveResizeWindow (server.dsp, traywin->id, traywin->x,
			 traywin->y, traywin->width, traywin->height);
      XResizeWindow (server.dsp, traywin->tray_id, traywin->width,
		    traywin->height);
      panel_refresh = 1;
      return;
    }
  }

  // 'win' move in another monitor
  if (nb_panel > 1 || hide_task_diff_monitor) {
    Task *tsk = task_get_task (win);
    if (tsk) {
      Panel *p = tsk->area.panel;
      int monitor = window_get_monitor (win);
      if ((hide_task_diff_monitor && p->monitor != monitor
	   && tsk->area.visible) ||
	  (hide_task_diff_monitor && p->monitor == monitor
	   && !tsk->area.visible) ||
	  (p->monitor != monitor && nb_panel > 1)) {
	remove_task (tsk);
	tsk = add_task (win);
	if (win == window_get_active ()) {
	  set_task_state (tsk, TASK_ACTIVE);
	  task_active = tsk;
	}
	panel_refresh = 1;
      }
    }
  }

  sort_taskbar_for_win (win);
}

char*
GetAtomName (Display* disp, Atom a) {
  if (a == None) return "None";
  else return XGetAtomName (disp, a);
}

//This fetches all the data from a property
Property read_property(Display* disp, Window w,
			      Atom property) {
  Atom actual_type;
  int actual_format;
  unsigned long nitems;
  unsigned long bytes_after;
  unsigned char *ret=0;

  int read_bytes = 1024;

  //Keep trying to read the property until there are no
  //bytes unread.
  do {
    if (ret != 0)
      XFree(ret);
    XGetWindowProperty(disp, w, property, 0, read_bytes, False,
		       AnyPropertyType, &actual_type, &actual_format,
		       &nitems, &bytes_after, &ret);
    read_bytes *= 2;
  } while (bytes_after != 0);

  return (Property){
    .data = ret,
    .nitems = nitems,
    .format = actual_format,
    .type = actual_type,
  };
}

// This function takes a list of targets which can be converted
// to (atom_list, nitems) and a list of acceptable targets with
// prioritees (datatypes). It returns the highest entry in
// datatypes which is also in atom_list: ie it finds the best match.
Atom
pick_target_from_list(Display* disp, Atom* atom_list,
			   int nitems) {
  Atom to_be_requested = None;
  int i;
  for (i = 0; i < nitems; i++) {
    char *atom_name = GetAtomName(disp, atom_list[i]);

    //See if this data type is allowed and of higher priority (closer to zero)
    //than the present one.
    if (strcmp(atom_name, "STRING") == 0) {
      to_be_requested = atom_list[i];
    }
  }

  return to_be_requested;
}

// Finds the best target given up to three atoms provided (any can be None).
// Useful for part of the Xdnd protocol.
Atom
pick_target_from_atoms(Display* disp, Atom t1, Atom t2,
			    Atom t3) {
  Atom atoms[3];
  int n = 0;

  if (t1 != None)
    atoms[n++] = t1;

  if (t2 != None)
    atoms[n++] = t2;

  if (t3 != None)
    atoms[n++] = t3;

  return pick_target_from_list(disp, atoms, n);
}

// Finds the best target given a local copy of a property.
Atom
pick_target_from_targets(Display* disp, Property p) {
  //The list of targets is a list of atoms, so it should have
  // type XA_ATOM but it may have the type TARGETS instead.

  if ((p.type != XA_ATOM && p.type != server.atom.TARGETS)
      || p.format != 32) {
    //This would be really broken. Targets have to be an atom list
    //and applications should support this. Nevertheless, some
    //seem broken (MATLAB 7, for instance), so ask for STRING
    //next instead as the lowest common denominator
    return XA_STRING;
  }
  else {
    Atom *atom_list = (Atom*)p.data;

    return pick_target_from_list (disp, atom_list, p.nitems);
  }
}


void
dnd_enter(XClientMessageEvent *e) {
  dnd_atom = None;
  int more_than_3 = e->data.l[1] & 1;
  dnd_source_window = e->data.l[0];
  dnd_version = (e->data.l[1] >> 24);

  //Query which conversions are available and pick the best

  if (more_than_3) {
    //Fetch the list of possible conversions
    //Notice the similarity to TARGETS with paste.
    Property p = read_property (server.dsp, dnd_source_window,
				server.atom.XdndTypeList);
    dnd_atom = pick_target_from_targets (server.dsp, p);
    XFree (p.data);
  }
  else {
    //Use the available list
    dnd_atom = pick_target_from_atoms(server.dsp, e->data.l[2],
				      e->data.l[3], e->data.l[4]);
  }
}

void
dnd_position(XClientMessageEvent *e) {
  dnd_target_window = e->window;
  int accept = 0;
  Panel *panel = panel_get (e->window);
  int x, y, mapX, mapY;
  Window child;
  x = (e->data.l[2] >> 16) & 0xFFFF;
  y = e->data.l[2] & 0xFFFF;
  XTranslateCoordinates (server.dsp, server.root_win,
			 e->window, x, y, &mapX, &mapY, &child);
  Task* task = panel_click_task (panel, (point_t){ mapX, mapY});
  if (task) {
    if (task->desktop != (uint32_t)server.desktop )
      set_desktop (task->desktop);
    window_action (task, TOGGLE);
  } else {
    LauncherIcon *icon = panel_click_launcher_icon (panel,
						    (point_t){mapX, mapY});
    if (icon) {
      accept = 1;
      dnd_launcher_exec = icon->cmd;
    }
    else dnd_launcher_exec = 0;
  }

  // send XdndStatus event to get more XdndPosition events
  XClientMessageEvent se;
  se.type = ClientMessage;
  se.window = e->data.l[0];
  se.message_type = server.atom.XdndStatus;
  se.format = 32;
  // XID of the target window
  se.data.l[0] = e->window;
  // bit 0: accept drop    bit 1: send XdndPosition events
  // if inside rectangle
  se.data.l[1] = accept ? 1 : 0;
  // Rectangle x,y for which no more XdndPosition events
  se.data.l[2] = 0;
  // Rectangle w,h for which no more XdndPosition events
  se.data.l[3] = (1 << 16) | 1;
  if (accept) {
    se.data.l[4] = dnd_version >= 2 ? e->data.l[4]
      : server.atom.XdndActionCopy;
  }
  else se.data.l[4] = None;       // None = drop will not be accepted


  XSendEvent (server.dsp, e->data.l[0], False, NoEventMask,
	      (XEvent*)&se);
}

void
dnd_drop (XClientMessageEvent *e) {
  if (dnd_target_window && dnd_launcher_exec) {
    if (dnd_version >= 1) {
      XConvertSelection (server.dsp, server.atom.XdndSelection,
			 XA_STRING, dnd_selection, dnd_target_window,
			 e->data.l[2]);
    }
    else
      XConvertSelection (server.dsp, server.atom.XdndSelection,
			 XA_STRING, dnd_selection, dnd_target_window,
			 CurrentTime);
  }
  else {
    //The source is sending anyway, despite instructions to the contrary.
    //So reply that we're not interested.
    XClientMessageEvent m;
    memset (&m, 0, sizeof m);
    m.type = ClientMessage;
    m.display = e->display;
    m.window = e->data.l[0];
    m.message_type = server.atom.XdndFinished;
    m.format = 32;
    m.data.l[0] = dnd_target_window;
    m.data.l[1] = 0;
    m.data.l[2] = None; //Failed.
    XSendEvent (server.dsp, e->data.l[0], False, NoEventMask,
		(XEvent*)&m);
  }
}
