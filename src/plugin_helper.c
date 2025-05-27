#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <signal.h>

#include "plugin.h"
#include "plugin_helper.h"

static pid_t *global_pids = NULL;
static unsigned int global_plugin_count = 0;

static void forward_sigusr1(int signo)
{
    (void)signo; // Unused parameter

    printf("[parent] Received SIGINT, forwarding SIGUSR1 to children\n");
    fflush(stdout);
    for (unsigned int i = 0; i < global_plugin_count; ++i)
    {
        if (global_pids[i] > 0)
        {
            printf("[parent] Sending SIGUSR1 to pid %d\n", global_pids[i]);
            fflush(stdout);
            kill(global_pids[i], SIGUSR1);
        }
    }
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

void init_plugins(char **plugin_file_names, const unsigned int plugin_count)
{
    printf("\nLoading plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_file_names[i], "init", "Loaded and initialized");
    }
}

void run_plugins(char **plugin_file_names, const unsigned int plugin_count)
{
    printf("\nRunning plugins...\n");

    // Block SIGINT in parent before forking
    sigset_t blockset, oldset;
    sigemptyset(&blockset);
    sigaddset(&blockset, SIGINT);
    sigprocmask(SIG_BLOCK, &blockset, &oldset);

    pid_t *pids = (pid_t *)calloc(plugin_count, sizeof(pid_t));
    if (!pids)
    {
        perror("ERROR: calloc failed");
        for (unsigned int i = 0; i < plugin_count; ++i)
        {
            handle_plugin_action(plugin_file_names[i], "run", "ran");
        }

        return;
    }

    global_pids = pids;
    global_plugin_count = plugin_count;

    // Set up SIGINT handler in parent
    struct sigaction sa;
    sa.sa_handler = forward_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Set the process name
            char path[256];
            snprintf(path, sizeof(path), "./build/plugins/%s.so", plugin_file_names[i]);
            void *plugin_handle = dlopen(path, RTLD_LAZY);
            if (!plugin_handle)
            {
                fprintf(stderr, "ERROR: dlopen failed for %s: %s\n", path, dlerror());
                return;
            }

            Plugin *plugin_struct = (Plugin *)dlsym(plugin_handle, plugin_file_names[i]);
            if (!plugin_struct)
            {
                fprintf(stderr, "ERROR: dlsym failed to find Plugin struct '%s' in %s: %s\n", plugin_file_names[i], path, dlerror());
                dlclose(plugin_handle);
                return;
            }

            char process_name[256];
            snprintf(process_name, sizeof(process_name), "%s", plugin_struct->name);
            prctl(PR_SET_NAME, process_name, 0, 0, 0);

            setpgid(0, 0);           // Child: move to its own process group, so it can handle signals independently
            signal(SIGINT, SIG_IGN); // Ignore SIGINT in child, as it will be handled by the parent
            sigprocmask(SIG_SETMASK, &oldset, NULL);
            handle_plugin_action(plugin_file_names[i], "run", "ran");
            exit(0);
        }
        else if (pid > 0)
        {
            global_pids[i] = pid;
            printf("[parent] Forked child %d for plugin %s\n", pid, plugin_file_names[i]);
        }
        else
        {
            perror("ERROR: fork failed");
        }
    }

    // Unblock SIGINT in parent after forking
    sigprocmask(SIG_SETMASK, &oldset, NULL);

    // Wait for all children
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        if (pids[i] > 0)
        {
            waitpid(pids[i], NULL, 0);
            printf("\tPlugin %s finished running\n", plugin_file_names[i]);
        }
    }

    // Restore default signal handler
    signal(SIGINT, SIG_DFL);

    free(pids);
    global_pids = NULL;
    global_plugin_count = 0;
}

void cleanup_plugins(char **plugin_file_names, const unsigned int plugin_count)
{
    printf("\nCleaning up plugins...\n");
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        handle_plugin_action(plugin_file_names[i], "cleanup", "Cleaned up");
    }
}

void free_plugin_file_names(char **plugin_file_names, const unsigned int plugin_count)
{
    for (unsigned int i = 0; i < plugin_count; ++i)
    {
        free(plugin_file_names[i]);
    }

    free(plugin_file_names);
}
