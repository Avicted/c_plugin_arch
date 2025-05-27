#ifndef PLUGIN_H
#define PLUGIN_H

typedef struct
{
    const char *name;
    void (*init)();
    void (*run)();
    void (*cleanup)();
} Plugin;

#endif // PLUGIN_H
