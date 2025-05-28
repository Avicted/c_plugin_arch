#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <signal.h>

#include "plugin.h"
#include "plugin_helper.h"

static Plugin *global_plugin_instances = NULL;
static unsigned int global_plugin_count = 0;
static char **plugin_file_names = NULL;

int set_process_id(void)
{
    int result = 0;
    if (isatty(STDIN_FILENO))
    {
        // Set the parent as its own process group leader.
        // This ensures that only the parent receives SIGINT (Ctrl+C) from the terminal,
        // and not the child plugin processes.
        pid_t parent_pid = getpid();
        if (setpgid(parent_pid, parent_pid) == -1)
        {
            perror("ERROR: setpgid failed");
            result = 3;
        }

        // Set the parent as the foreground process group for the terminal.
        // This is required so that the terminal sends signals (like SIGINT) to the parent.
        if (tcsetpgrp(STDIN_FILENO, parent_pid) == -1)
        {
            perror("ERROR: tcsetpgrp failed");
            result = 4;
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Not running in a real terminal. Skipping process group setup.\n");
    }

    return result;
}

void forward_sigusr1(int signo)
{
    (void)signo; // Unused parameter

    // This handler is called when the parent receives SIGINT (Ctrl+C).
    // It forwards SIGUSR1 to all child plugin processes, asking them to terminate gracefully.
    printf("[parent] Received SIGINT, forwarding SIGUSR1 to children\n");
    fflush(stdout);
    for (unsigned int i = 0; i < global_plugin_count; ++i)
    {
        if (global_plugin_instances[i].pid > 0)
        {
            printf("[parent] Sending SIGUSR1 to pid %d\n", global_plugin_instances[i].pid);
            fflush(stdout);
            kill(global_plugin_instances[i].pid, SIGUSR1);
        }
    }
}

time_t get_mtime(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        return st.st_mtime;
    }

    return 0;
}

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

    plugin_file_names = (char **)calloc(capacity, sizeof(char *));
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

        const char *file_extension = ".so";
        short file_extension_len = strlen(file_extension);
        if (len > 2 && strcmp(name + len - file_extension_len, file_extension) == 0)
        {
            // Remove ".so" to get the base name
            size_t base_len = len - file_extension_len;
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

    if (!plugin_file_names)
    {
        fprintf(stderr, "No plugins found or error occurred.\n");
        return NULL;
    }

    printf("Found %u plugins:\n", *plugin_count_out);
    for (unsigned int i = 0; i < *plugin_count_out; ++i)
    {
        printf("  - %s\n", plugin_file_names[i]);
    }

    return plugin_file_names;
}

void handle_plugin_action(Plugin *plugin_instance, PluginAction action, PluginState state)
{
    void (*plugin_func)(void) = NULL;
    const char *action_desc = plugin_action_names[action];
    const char *state_desc = plugin_state_names[state];

    if (!plugin_instance)
    {
        fprintf(stderr, "ERROR: Plugin instance is NULL\n");
        return;
    }

    switch (action)
    {
    case PLUGIN_ACTION_INIT:
        plugin_func = plugin_instance->init;
        break;
    case PLUGIN_ACTION_RUN:
        plugin_func = plugin_instance->run;
        break;
    case PLUGIN_ACTION_CLEANUP:
        plugin_func = plugin_instance->cleanup;
        break;
    default:
        fprintf(stderr, "ERROR: Unknown plugin action (%d)\n", action);
        return;
    }

    if (plugin_func)
    {
        plugin_func();
        printf("\tSuccessfully %s plugin: %s (%s).\n", action_desc, plugin_instance->name, state_desc);
    }
    else
    {
        fprintf(stderr, "ERROR: Plugin '%s' does not implement action '%s'\n", plugin_instance->name, action_desc);
    }
}

void init_plugins(char **plugin_file_names, const unsigned int plugin_count)
{
    printf("\nLoading plugins...\n");

    Plugin *plugin_instances = (Plugin *)calloc(plugin_count, sizeof(Plugin));
    if (!plugin_instances)
    {
        perror("ERROR: calloc failed for plugin instances");
        return;
    }

    global_plugin_instances = plugin_instances;
    global_plugin_count = plugin_count;

    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        char path[256];
        snprintf(path, sizeof(path), "./build/plugins/%s.so", plugin_file_names[i]);
        void *plugin_handle = dlopen(path, RTLD_LAZY);
        if (!plugin_handle)
        {
            fprintf(stderr, "ERROR: dlopen failed for %s: %s\n", path, dlerror());
            continue;
        }

        Plugin *plugin_instance = (Plugin *)dlsym(plugin_handle, plugin_file_names[i]);
        if (!plugin_instance)
        {
            fprintf(stderr, "ERROR: dlsym failed to find Plugin struct '%s' in %s: %s\n", plugin_file_names[i], path, dlerror());
            dlclose(plugin_handle);
            continue;
        }

        if (!plugin_instance->init || !plugin_instance->run || !plugin_instance->cleanup)
        {
            fprintf(stderr, "ERROR: Plugin '%s' missing required function(s)\n", plugin_file_names[i]);
        }

        plugin_instance->dl_handle = plugin_handle;

        plugin_instance->last_modified = get_mtime(path);
        if (plugin_instance->last_modified == 0)
        {
            fprintf(stderr, "ERROR: Could not get last modified time for %s\n", path);
            dlclose(plugin_handle);
            continue;
        }

        global_plugin_instances[i].name = plugin_file_names[i];
        global_plugin_instances[i] = *plugin_instance;
        global_plugin_instances[i].pid = -1; // Initialize pid to -1 (not running)
        global_plugin_instances[i].dl_handle = plugin_handle;

        handle_plugin_action(&global_plugin_instances[i], PLUGIN_ACTION_INIT, PLUGIN_STATE_INITIALIZED);
    }
}

void run_plugins(char **plugin_file_names, const unsigned int plugin_count)
{
    printf("\nRunning plugins...\n");

    // Block SIGINT in the parent before forking.
    // This prevents the parent from being interrupted by SIGINT while forking children.
    sigset_t blockset, oldset;
    sigemptyset(&blockset);
    sigaddset(&blockset, SIGINT);
    sigprocmask(SIG_BLOCK, &blockset, &oldset);

    // Set up SIGINT handler in parent to forward SIGUSR1 to children.
    struct sigaction sa;
    sa.sa_handler = forward_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        Plugin *plugin_instance = &global_plugin_instances[i];
        pid_t pid = fork();

        if (pid == 0)
        {
            // Set child process name to plugin name for easier debugging.
            char process_name[256];
            snprintf(process_name, sizeof(process_name), "%s", plugin_instance->name);
            prctl(PR_SET_NAME, process_name, 0, 0, 0);

            // Move child to its own process group so it does not receive SIGINT from the terminal.
            setpgid(0, 0);

            // Ignore SIGINT in the child; only the parent should handle it.
            signal(SIGINT, SIG_IGN);

            // Unblock SIGINT in the child (inherited from parent).
            sigprocmask(SIG_SETMASK, &oldset, NULL);

            // Run the plugin's main logic. The plugin should handle SIGUSR1 to exit gracefully.
            handle_plugin_action(plugin_instance, PLUGIN_ACTION_RUN, PLUGIN_STATE_RUNNING);
            exit(0);
        }
        else if (pid > 0)
        {
            // Optionally set parent process name for clarity.
            prctl(PR_SET_NAME, "c_plugin_arch_main", 0, 0, 0);
            global_plugin_instances[i].pid = pid;
            printf("[parent] Forked child %d for plugin %s\n", pid, plugin_instance->name);
        }
        else
        {
            perror("ERROR: fork failed");
        }
    }

    // Unblock SIGINT in parent after forking all children.
    sigprocmask(SIG_SETMASK, &oldset, NULL);

    // Wait for all children to finish.
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        if (global_plugin_instances[i].pid > 0)
        {
            if (waitpid(global_plugin_instances[i].pid, NULL, 0) == -1)
            {
                perror("waitpid failed");
            }

            printf("\tPlugin %s finished running\n", plugin_file_names[i]);
        }
    }

    // Restore default signal handler in parent.
    signal(SIGINT, SIG_DFL);
}

void cleanup_plugins(const unsigned int plugin_count)
{
    printf("\nCleaning up plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(&global_plugin_instances[i], PLUGIN_ACTION_CLEANUP, PLUGIN_STATE_TERMINATED);
    }
}

void free_plugins(const unsigned int plugin_count)
{
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        if (global_plugin_instances[i].dl_handle)
        {
            dlclose(global_plugin_instances[i].dl_handle);
        }
    }

    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        free(plugin_file_names[i]);
    }

    free(plugin_file_names);

    free(global_plugin_instances);
    global_plugin_instances = NULL;
    global_plugin_count = 0;
}
