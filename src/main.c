#include "plugin_helper.h"

int main(void)
{
    size_t plugin_count = 0;
    char **plugin_file_names = find_plugins("build/plugins", &plugin_count);

    if (!plugin_file_names)
    {
        fprintf(stderr, "ERROR: No plugin file names found.\n");
        free_plugins(plugin_count);
        return 1;
    }
    if (plugin_count <= 0)
    {
        fprintf(stderr, "ERROR: No plugins found in the directory.\n");
        free_plugins(plugin_count);
        return 2;
    }

    int result = set_process_id();
    if (result != 0)
    {
        return result;
    }

    init_plugins(plugin_file_names, plugin_count);
    run_plugins(plugin_file_names, plugin_count);
    cleanup_plugins(plugin_count);
    free_plugins(plugin_count);

    return 0;
}
