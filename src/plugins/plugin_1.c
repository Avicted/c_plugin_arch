#define _POSIX_C_SOURCE 200809L
#include <signal.h>

#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>

#include "plugin.h"

static volatile sig_atomic_t stop_requested = 0;

static void handle_sigusr1(int signo)
{
    (void)signo; // Unused parameter
    printf("[child] Received SIGUSR1\n");
    fflush(stdout);
    stop_requested = 1;
}

void init(void)
{
    printf("\tTest Plugin 1 initialized\n");
}

void run(void)
{
    printf("\tTest Plugin 1 running\n");

    struct sigaction sa;
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    // Simulate some work in the plugin
    for (int i = 0; i < 10; ++i)
    {
        if (stop_requested)
        {
            printf("\tTest Plugin 1 received stop signal, exiting run early\n");
            break;
        }

        sleep(1);
        printf("\tTest Plugin 1 working... %d\n", i + 1);
    }
}

void cleanup(void)
{
    printf("\tTest Plugin 1 cleaned up\n");
}

Plugin plugin_1 = {
    .name = "Test Plugin 1",
    .init = init,
    .run = run,
    .cleanup = cleanup,
};
