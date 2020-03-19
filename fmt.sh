#!/bin/sh

clang-format-4.0 \
    -style=file \
    -i \
    src/tinyframe.c \
    src/tinyframe/tinyframe.h \
    src/test/*.c
