/**************************************************************************
* config :
* - parse config file in Panel struct.
*
* Check COPYING file for Copyright
*
**************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

extern char *config_path;
extern char *thumbnail_path;

void init_config();
void cleanup_config();
int  config_read_file (const char *path);
int  config_read ();
void save_config ();

#endif

