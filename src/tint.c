/**************************************************************************
*
* Tint2 panel
*
* Copyright (C) 2007 Pål Staurland (staura@gmail.com)
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

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>
#include <X11/extensions/Xdamage.h>
#include <Imlib2.h>
#include <signal.h>

#ifdef HAS_SN
#include <libsn/sn.h>
#include <sys/wait.h>
#endif // HAS_SN


#include "debug.h"
#include "server.h"
#include "window.h"
#include "config.h"
#include "task.h"
#include "taskbar.h"
#include "systraybar.h"
#include "launcher.h"
#include "panel.h"
#include "tooltip.h"
#include "timer.h"
#include "xsettings-client.h"

#include "tinto.h"

extern int pending_signal; // Defined in tinto.o translation unit.

extern Window dnd_source_window;
extern Window dnd_target_window;
extern int dnd_version;
extern Atom dnd_selection;
extern Atom dnd_atom;
extern int dnd_sent_request;
extern char *dnd_launcher_exec;

#ifdef HAS_SN
extern int sn_pipe_valid;
extern int sn_pipe[2];
extern int error_trap_depth;
#endif

int
main (int argc, char *argv[]) {
  XEvent e;
  XClientMessageEvent *ev;
  fd_set fdset;
  int x11_fd, i;
  Panel *panel;
  GSList *it;
  struct timeval* timeout;
  int hidden_dnd = 0;

 start:
  tinto_init (argc, argv);

  i = 0;
  if (config_path)
    i = config_read_file (config_path);
  else
    i = config_read ();
  if (!i) {
    tinto_deinit ();
    tinto_usage ();
  }



  panel_init ();
  if (snapshot_path) {
    tinto_take_snapshot (snapshot_path);
    tinto_deinit ();
    exit (0);
  }

  int damage_event, damage_error;
  XDamageQueryExtension (server.dsp, &damage_event, &damage_error);
  x11_fd = ConnectionNumber (server.dsp);
  XSync (server.dsp, False);

  // XDND initialization
  dnd_source_window = 0;
  dnd_target_window = 0;
  dnd_version = 0;
  dnd_selection = XInternAtom (server.dsp, "PRIMARY", 0);
  dnd_atom = None;
  dnd_sent_request = 0;
  dnd_launcher_exec = 0;

  //	sigset_t empty_mask;
  //	sigemptyset(&empty_mask);

  while (1) {
    if (panel_refresh) {
      panel_refresh = 0;

      for (i=0 ; i < nb_panel ; i++) {
	panel = &panel1[i];

	if (panel->is_hidden) {
	  XCopyArea (server.dsp, panel->hidden_pixmap, panel->main_win,
		    server.gc, 0, 0, panel->hidden_width,
		    panel->hidden_height, 0, 0);
	  XSetWindowBackgroundPixmap (server.dsp, panel->main_win,
				     panel->hidden_pixmap);
	}
	else {
	  if (panel->temp_pmap)
	    XFreePixmap (server.dsp, panel->temp_pmap);
	  panel->temp_pmap = XCreatePixmap (server.dsp, server.root_win,
					    panel->area.width,
					    panel->area.height,
					    server.depth);
	  rendering (panel);
	  XCopyArea (server.dsp, panel->temp_pmap, panel->main_win,
		     server.gc, 0, 0, panel->area.width,
		     panel->area.height, 0, 0);
	}
      }
      XFlush (server.dsp);

      panel = (Panel*)systray.area.panel;
      if (refresh_systray && panel && !panel->is_hidden) {
	refresh_systray = 0;
	// tint2 doen't draw systray icons. it just redraw background.
	XSetWindowBackgroundPixmap (server.dsp, panel->main_win,
				    panel->temp_pmap);
	// force icon's refresh
	refresh_systray_icon ();
      }
    }

    // thanks to AngryLlama for the timer
    // Create a File Description Set containing x11_fd
    FD_ZERO (&fdset);
    FD_SET (x11_fd, &fdset);
    int maxfd = x11_fd;
    if (sn_pipe_valid) {
      FD_SET (sn_pipe[0], &fdset);
      maxfd = maxfd < sn_pipe[0] ? sn_pipe[0] : maxfd;
    }
    update_next_timeout();
    if (next_timeout.tv_sec >= 0 && next_timeout.tv_usec >= 0)
      timeout = &next_timeout;
    else
      timeout = 0;

    // Wait for X Event or a Timer
    if (select (maxfd+1, &fdset, 0, 0, timeout) > 0) {
      if (FD_ISSET (sn_pipe[0], &fdset)) {
	char buffer[1];
	ssize_t wur = read (sn_pipe[0], buffer, 1);
	(void) wur;
	launcher_sigchld_handler_async ();
      }
      if (FD_ISSET (x11_fd, &fdset)) {
	while (XPending (server.dsp)) {
	  XNextEvent (server.dsp, &e);
#if HAS_SN
	  if (startup_notifications)
	    sn_display_process_event (server.sn_dsp, &e);
#endif // HAS_SN

	  panel = panel_get (e.xany.window);
	  if (panel && panel_autohide) {
	    if (e.type == EnterNotify)
	      panel_autohide_trigger_show(panel);
	    else if (e.type == LeaveNotify)
	      panel_autohide_trigger_hide (panel);
	    if (panel->is_hidden) {
	      if (e.type == ClientMessage && e.xclient.message_type
		  == server.atom.XdndPosition) {
		hidden_dnd = 1;
		panel_autohide_show (panel);
	      }
	      // discard further processing of this event because the
	      // panel is not visible yet
	      else continue;
	    }
	    else if (hidden_dnd && e.type == ClientMessage
		     && e.xclient.message_type == server.atom.XdndLeave) {
	      hidden_dnd = 0;
	      autohide_hide (panel);
	    }
	  }

	  switch (e.type) {
	  case ButtonPress:
	    tooltip_hide (0);
	    event_button_press (&e);
	    break;

	  case ButtonRelease:
	    event_button_release (&e);
	    break;

	  case MotionNotify: {
	    unsigned int button_mask = Button1Mask | Button2Mask
	      | Button3Mask | Button4Mask | Button5Mask;
	    if (e.xmotion.state & button_mask)
	      event_button_motion_notify (&e);

	    Panel* panel = panel_get (e.xmotion.window);
	    Area* area =
	      panel_click_area (panel,
				(point_t){e.xmotion.x, e.xmotion.y});
	    if (area->_get_tooltip_text)
	      tooltip_trigger_show (area, panel, &e);
	    else
	      tooltip_trigger_hide ();
	    break;
	  }

	  case LeaveNotify:
	    tooltip_trigger_hide ();
	    break;

	  case Expose:
	    event_expose (&e);
	    break;

	  case PropertyNotify:
	    event_property_notify (&e);
	    break;

	  case ConfigureNotify:
	    event_configure_notify (e.xconfigure.window);
	    break;

	  case ReparentNotify:
	    if (!systray_enabled)
	      break;
	    panel = (Panel*)systray.area.panel;
	    if (e.xany.window == panel->main_win) // reparented to us
	      break;
	    // FIXME: 'reparent to us' badly detected => disabled
	    break;
	  case UnmapNotify:
	  case DestroyNotify:
	    if (e.xany.window == server.composite_manager) {
	      // Stop real_transparency
	      pending_signal = SIGUSR1;
	      break;
	    }
	    if (e.xany.window == g_tooltip.window || !systray_enabled)
	      break;
	    for (it = systray.list_icons; it; it = g_slist_next(it)) {
	      if (((TrayWindow*)it->data)->tray_id == e.xany.window) {
		remove_icon((TrayWindow*)it->data);
		break;
	      }
	    }
	    break;

	  case ClientMessage:
	    ev = &e.xclient;
	    if (ev->data.l[1] == (long)server.atom._NET_WM_CM_S0) {
	      if (ev->data.l[2] == None)
		// Stop real_transparency
		pending_signal = SIGUSR1;
	      else
		// Start real_transparency
		pending_signal = SIGUSR1;
	    }
	    if (systray_enabled && e.xclient.message_type
		== server.atom._NET_SYSTEM_TRAY_OPCODE && e.xclient.format
		== 32 && e.xclient.window == net_sel_win) {
	      net_message (&e.xclient);
	    }
	    else if (e.xclient.message_type == server.atom.XdndEnter)
	      dnd_enter (&e.xclient);
	    else if (e.xclient.message_type == server.atom.XdndPosition)
	      dnd_position (&e.xclient);
	    else if (e.xclient.message_type == server.atom.XdndDrop)
	      dnd_drop(&e.xclient);

	    break;

	  case SelectionNotify:
	    {
	      Atom target = e.xselection.target;

	      if (e.xselection.property != None && dnd_launcher_exec) {
		Property prop = read_property(server.dsp,
					      dnd_target_window,
					      dnd_selection);

		//If we're being given a list of targets
		// (possible conversions)
		if (target == server.atom.TARGETS && !dnd_sent_request) {
		  dnd_sent_request = 1;
		  dnd_atom = pick_target_from_targets(server.dsp, prop);

		  if (dnd_atom == None) {
		    fprintf (stderr, "No matching datatypes.\n");
		  } else {
		    //Request the data type we are able to select
		    fprintf (stderr, "Now requsting type %s",
			    GetAtomName (server.dsp, dnd_atom));
		    XConvertSelection (server.dsp, dnd_selection, dnd_atom,
				       dnd_selection, dnd_target_window,
				       CurrentTime);
		  }
		} else if (target == dnd_atom) {
		  //Dump the binary data
		  fprintf(stderr, "DnD %s:%d: Data begins:\n",
			  __FILE__, __LINE__);
		  fprintf(stderr, "--------\n");
		  int i;
		  for (i = 0; i < prop.nitems * prop.format/8; i++)
		    fprintf(stderr, "%c", ((char*)prop.data)[i]);
		  fprintf(stderr, "--------\n");

		  int cmd_length = 0;
		  cmd_length += 1; // (
		  cmd_length += strlen(dnd_launcher_exec) + 1; // exec + space
		  cmd_length += 1; // open double quotes
		  for (i = 0; i < prop.nitems * prop.format/8; i++) {
		    char c = ((char*)prop.data)[i];
		    if (c == '\n') {
		      if (i < prop.nitems * prop.format/8 - 1) {
			// close double quotes, space, open double quotes
			cmd_length += 3;
		      }
		    } else if (c == '\r') {
		    } else {
		      cmd_length += 1; // 1 character
		      if (c == '`' || c == '$' || c == '\\') {
			cmd_length += 1; // escape with one backslash
		      }
		    }
		  }
		  cmd_length += 1; // close double quotes
		  cmd_length += 2; // &)
		  cmd_length += 1; // terminator

		  char *cmd = calloc (cmd_length, 1);
		  cmd[0] = '\0';
		  strcat (cmd, "(");
		  strcat (cmd, dnd_launcher_exec);
		  strcat (cmd, " \"");
		  for (i = 0; i < prop.nitems * prop.format/8; i++) {
		    char c = ((char*)prop.data)[i];
		    if (c == '\n') {
		      if (i < prop.nitems * prop.format/8 - 1) {
			strcat (cmd, "\" \"");
		      }
		    }
		    else if (c == '\r') {

		    } else {
		      if (c == '`' || c == '$' || c == '\\') {
			strcat (cmd, "\\");
		      }
		      char sc[2];
		      sc[0] = c;
		      sc[1] = '\0';
		      strcat (cmd, sc);
		    }
		  }
		  strcat (cmd, "\"");
		  strcat (cmd, "&)");
		  fprintf (stderr, "DnD %s:%d: Running command: %s\n", __FILE__, __LINE__, cmd);
		  tint_exec (cmd);
		  free (cmd);

		  // Reply OK.
		  XClientMessageEvent m;
		  memset (&m, 0, sizeof(m));
		  m.type = ClientMessage;
		  m.display = server.dsp;
		  m.window = dnd_source_window;
		  m.message_type = server.atom.XdndFinished;
		  m.format = 32;
		  m.data.l[0] = dnd_target_window;
		  m.data.l[1] = 1;
		  m.data.l[2] = server.atom.XdndActionCopy; //We only ever copy.
		  XSendEvent (server.dsp, dnd_source_window, False,
			      NoEventMask, (XEvent*)&m);
		  XSync (server.dsp, False);
		}

		XFree (prop.data);
	      }

	      break;
	    }

	  default:
	    if (e.type == XDamageNotify+damage_event) {
	      // union needed to avoid strict-aliasing warnings by gcc
	      union {
		XEvent e;
		XDamageNotifyEvent de;
	      } event_union = {.e=e};
	      TrayWindow *traywin;
	      GSList *l;
	      XDamageNotifyEvent* de = &event_union.de;
	      for (l = systray.list_icons; l ; l = l->next) {
		traywin = (TrayWindow*)l->data;
		if ( traywin->id == de->drawable ) {
		  systray_render_icon (traywin);
		  break;
		}
	      }
	    }
	  }
	}
      }
    }

    callback_timeout_expired ();

    if (pending_signal) {
      tinto_deinit ();
      if (pending_signal == SIGUSR1) {
	// restart tint2
	// SIGUSR1 used when : user's signal, composite manager stop/start or xrandr
	FD_CLR (x11_fd, &fdset); // not sure if needed
	goto start;
      }
      else {
	// SIGINT, SIGTERM, SIGHUP
	return 0;
      }
    }
  }
}
