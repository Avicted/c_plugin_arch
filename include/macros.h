#include <stdlib.h>

#define ASSERT(x)                                                                   \
    if (!(x))                                                                       \
    {                                                                               \
        printf("Assertion failed: %s, file %s, line %d\n", #x, __FILE__, __LINE__); \
        exit(1);                                                                    \
    }
