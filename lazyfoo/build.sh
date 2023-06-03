#!/usr/bin/env bash

FILE=build/app

if test -f "$FILE"; then
	rm build/app
fi

export LD_LIBRARY_PATH="/usr/local/lib"

g++ src/main.cpp -w -lSDL2 -lSDL2_image -lSDL2_ttf -o build/app

build/app
