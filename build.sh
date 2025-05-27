#!/bin/bash

set -xe

rm -rf build || true
mkdir -p build
mkdir -p build/plugins

CC=clang
CFLAGS="-std=c99 -g -O0 -Wall -Wextra -Werror"
INCLUDES="-Iinclude"
LDLIBS="-ldl"

# Compile all plugins to shared libraries
for srcfile in src/plugins/*.c; do
    plugin_name=$(basename "$srcfile" .c)
    $CC -fPIC -shared \
        -o build/plugins/${plugin_name}.so "$srcfile" \
        $CFLAGS $INCLUDES $LDLIBS
done

# Compile the main program
$CC  src/plugin_helper.c src/main.c -o build/main $CFLAGS $LDLIBS $INCLUDES
