#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

// Standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// POSIX headers
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>

#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Linux-specific headers
#include <linux/prctl.h>

#include "plugin.h"

void forward_sigusr1(int signo);
time_t get_mtime(const char *path);

char **find_plugins(const char *directory, unsigned int *plugin_count_out);
void handle_plugin_action(Plugin *plugin_instance, PluginAction action, PluginState state);
void free_plugins(const unsigned int plugin_count);

void init_plugins(char **plugin_file_names, const unsigned int plugin_count);
void run_plugins(char **plugin_file_names, const unsigned int plugin_count);
void cleanup_plugins(const unsigned int plugin_count);

#endif // PLUGIN_HELPER_H
