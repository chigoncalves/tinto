#include "conf.h"


#include <stdlib.h>

#include "debug.h"
#include "tinto.h"

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
