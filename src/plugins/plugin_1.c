#include <stdio.h>
#include <unistd.h>

#include "plugin.h"

void init()
{
    printf("\tTest Plugin 1 initialized\n");
}

void run()
{
    printf("\tTest Plugin 1 running\n");

    // Simulate some work
    for (int i = 0; i < 5; ++i)
    {
        // Simulate a delay
        sleep(1);
        printf("\tTest Plugin 1 working... %d\n", i + 1);
    }
}

void cleanup()
{
    printf("\tTest Plugin 1 cleaned up\n");
}

Plugin plugin_1 = {
    .name = "Test Plugin 1",
    .init = init,
    .run = run,
    .cleanup = cleanup,
};
