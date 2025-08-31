#!/bin/sh
cd src
emcc -O2 -Iinclude common.c interpreter.c parser.c emfns.c -o ../web/piagn.js \
	-sEXPORTED_FUNCTIONS=_init_piagn,_free_piagn,_run_piagn \
	-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,FS \
	-sALLOW_MEMORY_GROWTH=1 -sMODULARIZE -sEXPORT_NAME=Piagn \
