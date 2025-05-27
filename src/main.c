#include "plugin_helper.h"

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
    run_plugins(plugin_file_names, plugin_count);
    cleanup_plugins(plugin_file_names, plugin_count);
    free_plugin_file_names(plugin_file_names, plugin_count);

    return 0;
}