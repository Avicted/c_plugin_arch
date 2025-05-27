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
#include "macros.h"

static char **
find_plugins(const char *directory, unsigned int *plugin_count_out)
{
    DIR *dir = opendir(directory);
    if (!dir)
    {
        perror("ERROR: opendir failed");
        *plugin_count_out = 0;
        return NULL;
    }

    struct dirent *entry;
    unsigned int capacity = 8;
    unsigned int count = 0;

    char **plugin_names = (char **)calloc(capacity, sizeof(char *));
    if (!plugin_names)
    {
        perror("ERROR: calloc failed");
        closedir(dir);
        *plugin_count_out = 0;
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        const char *name = entry->d_name;
        size_t len = strlen(name);

        if (len > 2 && strcmp(name + len - 2, ".c") == 0)
        {
            // Remove ".c" to get the base name
            size_t base_len = len - 2;
            char *base_name = (char *)calloc(base_len + 1, sizeof(char));
            if (!base_name)
            {
                perror("ERROR: calloc failed");
                continue;
            }
            strncpy(base_name, name, base_len);
            base_name[base_len] = '\0';

            // Resize if needed
            if (count >= capacity)
            {
                capacity *= 2;
                char **new_list = realloc(plugin_names, capacity * sizeof(char *));
                if (!new_list)
                {
                    perror("ERROR: realloc failed");
                    break;
                }
                plugin_names = new_list;
            }

            plugin_names[count++] = base_name;
        }
    }

    closedir(dir);
    *plugin_count_out = count;
    return plugin_names;
}

static void
handle_plugin_action(const char *plugin_name, const char *symbol, const char *action_desc)
{
    char path[256];
    snprintf(path, sizeof(path), "./build/plugins/%s.so", plugin_name);
    void *plugin_handle = dlopen(path, RTLD_LAZY);
    if (!plugin_handle)
    {
        fprintf(stderr, "ERROR: dlopen failed for %s: %s\n", path, dlerror());
        return;
    }

    void (*plugin_func)(void) = dlsym(plugin_handle, symbol);
    if (plugin_func)
    {
        plugin_func();
        printf("\tSuccessfully %s plugin: %s\n", action_desc, plugin_name);
    }
    else
    {
        fprintf(stderr, "ERROR: dlsym failed for %s in %s: %s\n", symbol, path, dlerror());
    }

    dlclose(plugin_handle);
}

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