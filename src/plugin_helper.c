#include "plugin.h"
#include "plugin_helper.h"

char **find_plugins(const char *directory, unsigned int *plugin_count_out)
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

    if (strcmp(symbol, "init") == 0)
    {
        plugin_func = plugin_struct->init;
    }
    else if (strcmp(symbol, "run") == 0)
    {
        plugin_func = plugin_struct->run;
    }
    else if (strcmp(symbol, "cleanup") == 0)
    {
        plugin_func = plugin_struct->cleanup;
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