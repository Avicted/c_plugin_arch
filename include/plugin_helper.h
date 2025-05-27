#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

// Standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// POSIX headers
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>

#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>

// Linux-specific headers
#include <linux/prctl.h>

#include "plugin.h"

typedef void (*plugin_func_t)(void);

typedef struct
{
    const char *name;
    plugin_func_t func;
} PluginFuncEntry;

char **find_plugins(const char *directory, unsigned int *plugin_count_out);
void handle_plugin_action(const char *plugin_file_name, const char *symbol, const char *action_desc);
void free_plugin_file_names(char **plugin_file_names, const unsigned int plugin_count);

void init_plugins(char **plugin_file_names, const unsigned int plugin_count);
void run_plugins(char **plugin_file_names, const unsigned int plugin_count);
void cleanup_plugins(char **plugin_file_names, const unsigned int plugin_count);

#endif // PLUGIN_HELPER_H
