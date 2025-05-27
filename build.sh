#!/bin/bash

set -xe

rm -rf build || true
mkdir -p build
mkdir -p build/plugins

CC=clang
CFLAGS="-std=c99 -g -O0 -Wall -Wextra -Werror"
INCLUDES="-Iinclude"
LDLIBS="-ldl -lc -lm"

# Compile all plugins to shared libraries
$CC -fPIC -shared \
    -o build/plugins/plugin_1.so src/plugins/plugin_1.c \
    $CFLAGS $INCLUDES $LDLIBS

$CC -fPIC -shared \
    -o build/plugins/plugin_2.so src/plugins/plugin_2.c \
    $CFLAGS $INCLUDES $LDLIBS

# Compile the main program
$CC src/main.c -o build/main $CFLAGS $LDLIBS $INCLUDES
