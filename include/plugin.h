#ifndef PLUGIN_H
#define PLUGIN_H

typedef struct
{
    void (*init)(void);
    void (*run)(void);
    void (*cleanup)(void);

    const char *name;
    time_t last_modified;
    pid_t pid;
    void *dl_handle;
} Plugin;

typedef void (*plugin_func_t)(void);

typedef struct
{
    const char *name;
    plugin_func_t func;
} PluginFuncEntry;

#endif // PLUGIN_H
