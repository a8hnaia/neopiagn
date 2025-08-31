#include "interpreter.h"
#include "parser.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void read_entire_file(FILE* fp, strview* str) {
	for (;;) {
		str->ptr[str->len] = fgetc(fp);
		if (feof(fp)) {
			return;
		}
		str->len++;
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fputs("Provide a file to run.\n", stderr);
		return 1;
	}
	FILE* fp = fopen(argv[1], "r");
	if (!fp) {
		fputs("Cannot open file.\n", stderr);
		return 1;
	}
	strview src = {
		.ptr = calloc(1, 4 * 1024),
		.len = 0
	};
	read_entire_file(fp, &src);
	fclose(fp);
	PgnFunctions fns = init_functions();
	ParserError err = {0};
	parse_source(&err, src, fns);
	if (err.kind) {
		fprintf(stderr, "Parser error kind: %d, at: %lu:%lu\n",
				err.kind, err.position.y + 1, err.position.x + 1);
		return 1;
	}
	ProgramState state = init_program();
	srand(time(NULL));
	run_program(&state, fns);
	free_program(&state);
	free_functions(&fns);
	free(src.ptr);
}
