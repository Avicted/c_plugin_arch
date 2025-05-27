#ifndef PLUGIN_H
#define PLUGIN_H

typedef struct
{
    const char *name;
    void (*init)(void);
    void (*run)(void);
    void (*cleanup)(void);
} Plugin;

#endif // PLUGIN_H
