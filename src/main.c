#include "plugin_helper.h"

static char **
find_and_print_plugins(const char *plugin_dir, unsigned int *plugin_count)
{
    char **plugin_file_names = find_plugins(plugin_dir, plugin_count);

    if (!plugin_file_names)
    {
        fprintf(stderr, "No plugins found or error occurred.\n");
        return NULL;
    }

    printf("Found %u plugins:\n", *plugin_count);
    for (unsigned int i = 0; i < *plugin_count; ++i)
    {
        printf("  - %s\n", plugin_file_names[i]);
    }
    return plugin_file_names;
}

static void
load_plugins(char **plugin_file_names, unsigned int plugin_count)
{
    printf("\nLoading plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_file_names[i], "init", "Loaded and initialized");
    }
}

static void
run_plugins(char **plugin_file_names, unsigned int plugin_count)
{
    printf("\nRunning plugins...\n");
    pid_t *pids = (pid_t *)calloc(plugin_count, sizeof(pid_t));
    if (!pids)
    {
        perror("ERROR: calloc failed");

        // fallback to sequential execution
        for (unsigned int i = 0; i < plugin_count; ++i)
        {
            handle_plugin_action(plugin_file_names[i], "run", "ran");
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
                handle_plugin_action(plugin_file_names[i], "run", "ran");
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
}

static void
cleanup_plugins(char **plugin_file_names, unsigned int plugin_count)
{
    printf("\nCleaning up plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_file_names[i], "cleanup", "Cleaned up");
    }
}

static void
free_plugin_file_names(char **plugin_file_names, unsigned int plugin_count)
{
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        free(plugin_file_names[i]);
    }
    free(plugin_file_names);
}

int main(void)
{
    unsigned int plugin_count = 0;
    char **plugin_file_names = find_and_print_plugins("src/plugins", &plugin_count);

    if (!plugin_file_names)
    {
        return 1;
    }

    load_plugins(plugin_file_names, plugin_count);
    run_plugins(plugin_file_names, plugin_count);
    cleanup_plugins(plugin_file_names, plugin_count);
    free_plugin_file_names(plugin_file_names, plugin_count);

    return 0;
}