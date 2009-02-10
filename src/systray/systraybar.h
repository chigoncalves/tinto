/**************************************************************************
* Copyright (C) 2009 thierry lorthiois (lorthiois@bbsoft.fr)
*
* systraybar
*
**************************************************************************/

#ifndef SYSTRAYBAR_H
#define SYSTRAYBAR_H

#include "area.h"


typedef struct {
   // always start with area
   Area area;

} Systraybar;


typedef struct
{
  Window id;
  int x, y;
} TrayWindow;


void init_systray(Systraybar *sysbar, Area *parent);

// return 1 if task_width changed
int resize_systray (Systraybar *sysbar);



#endif

