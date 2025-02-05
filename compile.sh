#!/bin/bash
set -e

if [ ! -f "thecat.h" ]; then
	xxd -i ./cat.png > thecat.h
fi
x86_64-w64-mingw32-gcc -o cat.exe catfunny.c -lm
