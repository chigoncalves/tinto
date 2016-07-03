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
#include "panel.h"
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

  if (panel->area.width > server.monitor[0].width)
    panel->area.width = server.monitor[0].width;

  panel->temp_pmap = XCreatePixmap (server.dsp, server.root_win,
				    panel->area.width,
				    panel->area.height,
				    server.depth);
  rendering (panel);

  imlib_context_set_drawable (panel->temp_pmap);
  Imlib_Image img =
    imlib_create_image_from_drawable (None, 0, 0,
				      panel->area.width,
				      panel->area.height, 0);

  imlib_context_set_image (img);
  if (!panel_horizontal) {
    imlib_image_flip_horizontal ();
    imlib_image_flip_diagonal ();
  }
  imlib_save_image (path);
  imlib_free_image ();
}
