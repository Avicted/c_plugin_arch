#include <stdio.h>
#include <unistd.h>

#include "plugin.h"

void init()
{
    printf("\tTest Plugin 2 initialized\n");
}

void run()
{
    printf("\tTest Plugin 2 running\n");

    // Simulate some work
    for (int i = 0; i < 5; ++i)
    {
        // Simulate a delay
        sleep(1);
        printf("\tTest Plugin 2 working... %d\n", i + 1);
    }
}

void cleanup()
{
    printf("\tTest Plugin 2 cleaned up\n");
}

Plugin plugin_2 = {
    .name = "Test Plugin 2",
    .init = init,
    .run = run,
    .cleanup = cleanup,
};
