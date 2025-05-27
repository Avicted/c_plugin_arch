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
    printf("\tTest Plugin 2 initialized\n");

    struct sigaction sa;
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}

void run(void)
{
    printf("\tTest Plugin 2 running\n");

    // Simulate some work in the plugin
    for (int i = 0; i < 10; ++i)
    {
        if (stop_requested)
        {
            printf("\tTest Plugin 2 received stop signal, exiting run early\n");
            break;
        }

        sleep(1);
        printf("\tTest Plugin 2 working... %d\n", i + 1);
    }
}

void cleanup(void)
{
    printf("\tTest Plugin 2 cleaned up\n");
}

Plugin plugin_2 = {
    .name = "Test Plugin 2",
    .init = init,
    .run = run,
    .cleanup = cleanup,
};
