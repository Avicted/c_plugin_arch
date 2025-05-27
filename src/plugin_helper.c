#include "plugin.h"
#include "plugin_helper.h"

char **find_plugins(const char *directory, unsigned int *plugin_count_out)
{
    DIR *dir = opendir(directory);
    if (!dir)
    {
        fprintf(stderr, "ERROR: opendir failed. Directory %s not found\n", directory);
        *plugin_count_out = 0;
        return NULL;
    }

    struct dirent *entry;
    unsigned int capacity = 8;
    unsigned int count = 0;

    char **plugin_file_names = (char **)calloc(capacity, sizeof(char *));
    if (!plugin_file_names)
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
                char **new_list = realloc(plugin_file_names, capacity * sizeof(char *));
                if (!new_list)
                {
                    perror("ERROR: realloc failed");
                    break;
                }
                plugin_file_names = new_list;
            }

            plugin_file_names[count++] = base_name;
        }
    }

    closedir(dir);
    *plugin_count_out = count;
    return plugin_file_names;
}

void handle_plugin_action(const char *plugin_file_name, const char *symbol, const char *action_desc)
{
    char path[256];
    snprintf(path, sizeof(path), "./build/plugins/%s.so", plugin_file_name);
    void *plugin_handle = dlopen(path, RTLD_LAZY);
    if (!plugin_handle)
    {
        fprintf(stderr, "ERROR: dlopen failed for %s: %s\n", path, dlerror());
        return;
    }

    // Use the Plugin struct from the plugin .so
    Plugin *plugin_struct = (Plugin *)dlsym(plugin_handle, plugin_file_name);
    if (!plugin_struct)
    {
        fprintf(stderr, "ERROR: dlsym failed to find Plugin struct '%s' in %s: %s\n", plugin_file_name, path, dlerror());
        dlclose(plugin_handle);
        return;
    }

    const char *plugin_name = plugin_struct->name;
    void (*plugin_func)(void) = NULL;

    PluginFuncEntry plugin_funcs[] = {
        {"init", plugin_struct->init},
        {"run", plugin_struct->run},
        {"cleanup", plugin_struct->cleanup},
    };

    plugin_func = NULL;
    for (int i = 0; plugin_funcs[i].name != NULL; ++i)
    {
        if (strcmp(symbol, plugin_funcs[i].name) == 0)
        {
            plugin_func = plugin_funcs[i].func;
            break;
        }
    }

    if (plugin_func)
    {
        plugin_func();
        printf("\tSuccessfully %s plugin: %s\n\n", action_desc, plugin_name);
    }
    else
    {
        fprintf(stderr, "ERROR: Plugin struct for %s does not have function '%s'\n", plugin_name, symbol);
    }

    dlclose(plugin_handle);
}

char **find_and_print_plugins(const char *plugin_dir, unsigned int *plugin_count)
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

void load_plugins(char **plugin_file_names, unsigned int plugin_count)
{
    printf("\nLoading plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_file_names[i], "init", "Loaded and initialized");
    }
}

void run_plugins(char **plugin_file_names, unsigned int plugin_count)
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

void cleanup_plugins(char **plugin_file_names, unsigned int plugin_count)
{
    printf("\nCleaning up plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_file_names[i], "cleanup", "Cleaned up");
    }
}

void free_plugin_file_names(char **plugin_file_names, unsigned int plugin_count)
{
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        free(plugin_file_names[i]);
    }
    free(plugin_file_names);
}
