#!/bin/sh
mkdir -p bin
cd src
gcc -O2 -Iinclude common.c interpreter.c parser.c main.c -o ../bin/piagn
