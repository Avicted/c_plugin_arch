#include "plugin_helper.h"
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    unsigned int plugin_count = 0;
    char **plugin_file_names = find_and_print_plugins("src/plugins", &plugin_count);

    if (!plugin_file_names)
    {
        return 1;
    }
    if (plugin_count <= 0)
    {
        fprintf(stderr, "No plugins found. Exiting.\n");
        free_plugin_file_names(plugin_file_names, plugin_count);
        return 2;
    }

    load_plugins(plugin_file_names, plugin_count);

    // Set up signal handling for SIGINT: make parent the foreground process group
    pid_t parent_pid = getpid();
    if (setpgid(parent_pid, parent_pid) == -1)
    {
        perror("ERROR: setpgid failed, Not running in a real terminal. Some features may not work.\n");
        return 3;
    }
    if (tcsetpgrp(STDIN_FILENO, parent_pid) == -1)
    {
        perror("ERROR: tcsetpgrp failed, Not running in a real terminal. Some features may not work.\n");
        return 4;
    }

    run_plugins(plugin_file_names, plugin_count);
    cleanup_plugins(plugin_file_names, plugin_count);
    free_plugin_file_names(plugin_file_names, plugin_count);

    return 0;
}