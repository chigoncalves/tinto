/**************************************************************************
 * Copyright (C) 2015       (mrovi9000@gmail.com)
 *
 *
 **************************************************************************/

#ifndef APPS_COMMON_H
#define APPS_COMMON_H

// For `bool', `true' and `false'.
#include <stdbool.h>

typedef struct {
  char *name;
  char *exec;
  char *icon;
} desktop_entry_t;

// Parses a line of the form "key = value". Modifies the line.
// Returns 1 if successful, and parts are not empty.
// Key and value point to the parts.
bool
desktop_entry_parse_desktop_line (char *line, char **key,
				  char **value);

// Reads the .desktop file from the given path into the DesktopEntry entry.
// The DesktopEntry object must be initially empty.
// Returns 1 if successful.

desktop_entry_t*
desktop_entry_create (const char* path);

// Empties DesktopEntry: releases the memory of the *members* of entry.
void
desktop_entry_destroy (desktop_entry_t entry[static 1]);

#endif
