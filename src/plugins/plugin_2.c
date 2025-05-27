#include <stdio.h>
#include <unistd.h>

#include "plugin.h"

void init()
{
    printf("Plugin 2 initialized\n");
}

void run()
{
    printf("Plugin 2 running\n");

    // Simulate some work
    for (int i = 0; i < 5; ++i)
    {
        // Simulate a delay
        sleep(1);
        printf("Plugin 2 working... %d\n", i + 1);
    }
}

void cleanup()
{
    printf("Plugin 2 cleaned up\n");
}
