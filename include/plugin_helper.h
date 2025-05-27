#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <signal.h>
#include <linux/prctl.h>
#include <sys/prctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "plugin.h"

typedef void (*plugin_func_t)(void);

typedef struct
{
    const char *name;
    plugin_func_t func;
} PluginFuncEntry;

char **find_plugins(const char *directory, unsigned int *plugin_count_out);
void handle_plugin_action(const char *plugin_file_name, const char *symbol, const char *action_desc);

char **find_and_print_plugins(const char *plugin_dir, unsigned int *plugin_count);
void load_plugins(char **plugin_file_names, const unsigned int plugin_count);
void run_plugins(char **plugin_file_names, const unsigned int plugin_count);
void cleanup_plugins(char **plugin_file_names, const unsigned int plugin_count);
void free_plugin_file_names(char **plugin_file_names, const unsigned int plugin_count);

#endif // PLUGIN_HELPER_H