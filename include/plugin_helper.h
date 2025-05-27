#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "plugin.h"

char **find_plugins(const char *directory, unsigned int *plugin_count_out);
void handle_plugin_action(const char *plugin_name, const char *symbol, const char *action_desc);

#endif // PLUGIN_HELPER_H