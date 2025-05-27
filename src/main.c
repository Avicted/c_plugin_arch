#include "plugin_helper.h"
#include <unistd.h>
#include <stdio.h>

static int
set_process_id(void)
{
    int result = 0;
    if (isatty(STDIN_FILENO))
    {
        pid_t parent_pid = getpid();
        if (setpgid(parent_pid, parent_pid) == -1)
        {
            perror("setpgid failed");
            result = 3;
        }
        if (tcsetpgrp(STDIN_FILENO, parent_pid) == -1)
        {
            perror("tcsetpgrp failed");
            result = 4;
        }
    }
    else
    {
        fprintf(stderr, "Not running in a real terminal. Skipping process group setup.\n");
    }

    return result;
}

int main(void)
{
    unsigned int plugin_count = 0;
    char **plugin_file_names = find_and_print_plugins("build/plugins", &plugin_count);

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

    int result = set_process_id();
    if (result != 0)
    {
        return result;
    }

    init_plugins(plugin_file_names, plugin_count);
    run_plugins(plugin_file_names, plugin_count);
    cleanup_plugins(plugin_file_names, plugin_count);
    free_plugin_file_names(plugin_file_names, plugin_count);

    return 0;
}
