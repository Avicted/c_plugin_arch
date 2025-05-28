#ifndef PLUGIN_H
#define PLUGIN_H

typedef struct
{
    const char *name;
    time_t last_modified;
    pid_t pid;
    void *dl_handle;

    void (*init)(void);
    void (*run)(void);
    void (*cleanup)(void);
} Plugin;

typedef enum
{
    PLUGIN_ACTION_INIT,
    PLUGIN_ACTION_RUN,
    PLUGIN_ACTION_CLEANUP,
} PluginAction;

static const char *plugin_action_names[] = {
    "init",
    "ran",
    "cleanup",
};

typedef enum
{
    PLUGIN_STATE_INITIALIZED,
    PLUGIN_STATE_RUNNING,
    PLUGIN_STATE_TERMINATED,
} PluginState;

static const char *plugin_state_names[] = {
    "initialized",
    "ran to completion",
    "terminated",
};

#endif // PLUGIN_H
