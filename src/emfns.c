#include "parser.h"
#include "interpreter.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	PgnFunctions fns;
	ParserError err;
	ProgramState state;
} PgnData;

PgnData* init_piagn() {
	PgnData* pgn = calloc(1, sizeof(PgnData));
	pgn->fns = init_functions();
	pgn->state = init_program();
	return pgn;
}

void free_piagn(PgnData* pgn) {
	free_functions(&pgn->fns);
	free_program(&pgn->state);
	free(pgn);
}

void run_piagn(PgnData* pgn, char* src) {
	strview sv_src = lit_sv(src);
	parse_source(&pgn->err, sv_src, pgn->fns);
	ParserError err = pgn->err;
	if (err.kind) {
		fprintf(stderr, "Parser error kind: %d, at: %lu:%lu\n",
				err.kind, err.position.y + 1, err.position.x + 1);
		return;
	}
	srand(time(NULL));
	run_program(&pgn->state, pgn->fns);
}
