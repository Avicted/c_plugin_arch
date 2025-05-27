#include "plugin_helper.h"

int main(void)
{
    unsigned int plugin_count = 0;
    char **plugin_names = find_plugins("src/plugins", &plugin_count);

    if (!plugin_names)
    {
        fprintf(stderr, "No plugins found or error occurred.\n");
        return 1;
    }

    printf("Found %u plugins:\n", plugin_count);
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        printf("  - %s\n", plugin_names[i]);
    }
    printf("\nLoading plugins...\n");

    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_names[i], "init", "loaded and initialized");
    }

    printf("\nRunning plugins...\n");
    pid_t *pids = (pid_t *)calloc(plugin_count, sizeof(pid_t));
    if (!pids)
    {
        perror("ERROR: calloc failed");

        // fallback to sequential execution
        for (unsigned int i = 0; i < plugin_count; ++i)
        {
            handle_plugin_action(plugin_names[i], "run", "ran");
        }
    }
    else
    {
        for (unsigned int i = 0; i < plugin_count; ++i)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                // Child process
                handle_plugin_action(plugin_names[i], "run", "ran");
                exit(0);
            }
            else if (pid > 0)
            {
                // Parent process
                pids[i] = pid;
            }
            else
            {
                perror("ERROR: fork failed");
            }
        }

        // Wait for all children
        for (unsigned int i = 0; i < plugin_count; ++i)
        {
            if (pids[i] > 0)
            {
                waitpid(pids[i], NULL, 0);
            }
        }
        free(pids);
    }

    printf("\nCleaning up plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_names[i], "cleanup", "cleaned up");
    }

    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        free(plugin_names[i]);
    }
    free(plugin_names);

    return 0;
}