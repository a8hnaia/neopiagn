#include "interpreter.h"
#include "parser.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

strview read_entire_file(FILE* fp) {
	fseek(fp, 0, SEEK_END);
	strview str;
	str.len = ftell(fp);
	str.ptr = calloc(1, str.len);
	fseek(fp, 0, SEEK_SET);
	for (size_t i = 0; i < str.len; i++) {
		str.ptr[i] = fgetc(fp);
	}
	return str;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fputs("Provide a file to run.\n", stderr);
		return 1;
	}
	FILE* fp = fopen(argv[1], "rb");
	if (!fp) {
		fputs("Cannot open file.\n", stderr);
		return 1;
	}
	strview src = read_entire_file(fp);
	fclose(fp);
	PgnFunctions fns = init_functions();
	ParserError err = {0};
	parse_source(&err, src, fns);
	if (err.kind) {
		fprintf(stderr, "Parser error kind: %d, at: %lu:%lu\n",
				err.kind, err.position.y + 1, err.position.x + 1);
		free_functions(&fns);
		free(src.ptr);
		return 1;
	}
	ProgramState state = init_program();
	srand(time(NULL));
	run_program(&state, fns);
	free_program(&state);
	free_functions(&fns);
	free(src.ptr);
}
