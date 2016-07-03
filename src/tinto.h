#ifndef TINTO_SRC_TINTO_H
#define TINTO_SRC_TINTO_H 1

#include <stdnoreturn.h>

#include <X11/Xutil.h>

#include "panel.h"

typedef struct  {
  unsigned char *data;
  int format, nitems;
  Atom type;
} Property;


noreturn void
tinto_usage (void);

void
tinto_signal_handler(int sig);

void
tinto_init (int argc, char *argv[]);

void
tinto_deinit (void);

void
tinto_take_snapshot(const char *path);

int
tint2_handles_click(Panel* panel, XButtonEvent* e);

void
forward_click (XEvent* e);

void
event_button_press (XEvent *e);

void
event_button_motion_notify (XEvent *e);

void
event_button_release (XEvent *e);

void
event_property_notify (XEvent *e);

void
event_expose (XEvent *e);

void
event_configure_notify (Window win);

char*
GetAtomName (Display* disp, Atom a);

Property read_property(Display* disp, Window w,
		       Atom property);

Atom
pick_target_from_list(Display* disp, Atom* atom_list,
		      int nitems);

Atom
pick_target_from_atoms(Display* disp, Atom t1, Atom t2,
			    Atom t3);

Atom
pick_target_from_targets(Display* disp, Property p);

void
dnd_position(XClientMessageEvent *e);

void
dnd_enter(XClientMessageEvent *e);

void
dnd_drop (XClientMessageEvent *e);


#endif // TINTO_SRC_TINTO_H
