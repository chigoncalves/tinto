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
}


#if 0

#ifdef HAS_SN
  #include <glib.h>
  #include <unistd.h>
  #include <libsn/sn.h>
  #include <sys/wait.h>

  #include <string.h>

  #include "debug.h"
  #include "server.h"
#endif // HAS_SN

void
sigchld_handler_async (void) {
#ifdef HAS_SN
  if (!startup_notifications) return;

  // Wait for all dead processes
  pid_t pid;
  while ((pid = waitpid (-1, NULL, WNOHANG)) > 0) {
    SnLauncherContext *ctx =
      (SnLauncherContext *)g_tree_lookup (server.pids,
					  GINT_TO_POINTER (pid));


    if (ctx) {
      g_tree_remove (server.pids, GINT_TO_POINTER (pid));
      sn_launcher_context_complete (ctx);
      sn_launcher_context_unref (ctx);
    }
    else
      MSG ("Unknown child %d terminated!\n", pid);
  }
#endif // HAS_SN
}

#endif //
