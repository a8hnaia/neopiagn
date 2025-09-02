#include "parser.h"

#include <stdio.h>

typedef struct {
	SVec2 position;
	size_t index;
	Instruction current_function;
} ParserState;

static Instruction letter_to_function(char c);
static bool source_eof(strview src, ParserState* state);
static void next(strview src, ParserState* state);
static void parse_source_with_functions(ParserError* err, strview src, PgnFunctions fns);
static void parse_source_main_only(ParserError* err, strview src, PgnFunctions fns);
static void parse_function(ParserError* err, strview src, PgnFunctions fns, ParserState* state);
static Instruction unresolved_instruction(char c);
static void parse_function_body(ParserError* err, strview src, PgnFunctions fns, ParserState* state);
static void resolve_intructions(ParserError* err, strview src, PgnFunctions fns);
static void resolve_start(ParserError* err, strview src, PgnFunction* fn);
static void resolve_conditionals(ParserError* err, strview src, PgnFunction* fn);

void parse_source(ParserError* err, strview src, PgnFunctions fns) {
	// If there are no opening braces in the file, assume main only.
	bool with_function = false;
	for (size_t i = 0; i < src.len; i++) {
		if (src.ptr[i] == '{') {
			with_function = true;
			break;
		}
	}
	if (with_function) {
		parse_source_with_functions(err, src, fns);
	}
	else {
		parse_source_main_only(err, src, fns);
	}
	resolve_intructions(err, src, fns);
}

Instruction letter_to_function(char c) {
	if ('A' <= c && c <= 'Z') {
		return c - 'A' + I_UPPER_A;
	}
	else if ('a' <= c && c <= 'z') {
		return c - 'a' + I_LOWER_A;
	}
	// Because main is marked with `@`.
	else if (c == '@') {
		return I_MAIN;
	}
	// Not a function.
	else {
		return I_EMPTY;
	}
}

bool source_eof(strview src, ParserState* state) {
	return src.len <= state->index;
}

void next(strview src, ParserState* state) {
	state->index++;
	state->position.x++;
	while (!source_eof(src, state) && src.ptr[state->index] == ' ') {
		state->index++;
		state->position.x++;
	}
	while (!source_eof(src, state) && src.ptr[state->index] == '\n') {
		state->index++;
		state->position.y++;
		state->position.x = 0;
	}
}

void parse_source_with_functions(ParserError* err, strview src, PgnFunctions fns) {
	ParserState state = {0};
	while (!source_eof(src, &state)) {
		parse_function(err, src, fns, &state);
		if (err->kind) {
			return;
		}
		next(src, &state);
	}
}

void parse_source_main_only(ParserError* err, strview src, PgnFunctions fns) {
	ParserState state = {0};
	state.current_function = I_MAIN;
	parse_function_body(err, src, fns, &state);
	if (err->kind) {
		return;
	}
	if (!source_eof(src, &state)) {
		err->index = state.index;
		err->kind = PARSER_ERROR_IMPROPER_BRACE;
		err->position = state.position;
		return;
	}
}

void parse_function(ParserError* err, strview src, PgnFunctions fns, ParserState* state) {
	Instruction inst = letter_to_function(src.ptr[state->index]);
	state->current_function = inst;
	next(src, state);
	if (src.ptr[state->index] != '{') {
		err->index = state->index;
		err->kind = PARSER_ERROR_IMPROPER_BRACE;
		err->position = state->position;
		return;
	}
	next(src, state);
	parse_function_body(err, src, fns, state);
	if (err->kind) {
		return;
	}
	if (src.ptr[state->index] != '}') {
		err->index = state->index;
		err->kind = PARSER_ERROR_IMPROPER_BRACE;
		err->position = state->position;
		return;
	}
}

// Turns the char into an Instruction without resolving it into
// an actual instruction.
Instruction unresolved_instruction(char c) {
	switch (c) {
		case ' ': return I_EMPTY; break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return c - '0' + I_0;
			break;
		case '|': return I_COND_VT; break;
		case '-': return I_COND_HZ; break;
		case '*': return I_COND_TRI; break;
		case '/': return I_MIRROR_RIGHT; break;
		case '\\': return I_MIRROR_LEFT; break;
		case '#': return I_REPEAT; break;
		case '?': return I_RANDOM; break;
		case '!': return I_DUPLICATE; break;
		case '%': return I_SWAP; break;
		case '_': return I_DROP; break;
		case '^': return I_XOR; break;
		case '&': return I_AND; break;
		case '+': return I_OR; break;
		case '[': return I_SHIFT_LEFT; break;
		case ']': return I_SHIFT_RIGHT; break;
		case '~': return I_NOT; break;
		case '$': return I_GO_PILE; break;
		case '<': return I_LEFT_PILE; break;
		case '>': return I_RIGHT_PILE; break;
		case ',': return I_FROM_PILE; break;
		case '.': return I_TO_PILE; break;
		case ';': return I_READ; break;
		case ':': return I_WRITE; break;
		default: {
			Instruction inst = letter_to_function(c);
			if (inst) {
				return inst;
			}
			else {
				// As a placeholder for an invalid character.
				return I_HALT_VT;
			}
		} break;
	}
}

void parse_function_body(ParserError* err, strview src, PgnFunctions fns, ParserState* state) {
	size_t start_line = state->position.y;
	PgnFunction* fn = get_function(fns, state->current_function);
	while (!source_eof(src, state)) {
		char cur = src.ptr[state->index];
		SVec2 pos = {
			.x = state->position.x,
			.y = state->position.y - start_line
		};
		if (pos.x == 0 && cur == '}') {
			return;
		}
		else if (cur == '}') {
			err->index = state->index;
			err->kind = PARSER_ERROR_IMPROPER_BRACE;
			err->position = state->position;
			return;
		}
		Instruction inst = unresolved_instruction(cur);
		if (inst != I_HALT_VT) {
			accomodate_position(fn, pos);
			*get_instruction(fn, pos) = inst;
		}
		else {
			err->kind = PARSER_ERROR_INVALID_INSTRUCTION;
			err->index = state->index;
			err->position = state->position;
			return;
		}
		next(src, state);
	}
}

void resolve_intructions(ParserError* err, strview src, PgnFunctions fns) {
	for (Instruction i = I_UPPER_A; i <= I_MAIN; i++) {
		PgnFunction* fn = get_function(fns, i);
		resolve_start(err, src, fn);
		resolve_conditionals(err, src, fn);
	}
}

// Resolve the starting point of the function, i.e `@`.
void resolve_start(ParserError* err, strview src, PgnFunction* fn) {
	fn->start_direction = D_RIGHT;
	SVec2 pos = {0};
	bool at_found = false;
	for (pos.y = 0; pos.y < 256 * fn->size.y; pos.y++) {
		for (pos.x = 0; pos.x < 256 * fn->size.x; pos.x++) {
			Instruction* inst = get_instruction(fn, pos);
			if (*inst == I_MAIN) {
				if (at_found) {
					err->kind = PARSER_ERROR_INVALID_START;
					// TODO
					err->index = 0;
					err->position = (SVec2){0};
					return;
				}
				bool dir_found = false;
				for (Direction i = 0; i < 4; i++) {
					Instruction adj = instruction_option(get_instruction(fn,
							go_direction(pos, i)), I_EMPTY);
					if (adj && dir_found) {
						err->kind = PARSER_ERROR_INVALID_START;
						// TODO
						err->index = 0;
						err->position = (SVec2){0};
						return;
					}
					if (adj) {
						at_found = true;
						dir_found = true;
						fn->start_direction = i;
						fn->start_position = pos;
						*inst = I_EMPTY;
					}
				}
			}
		}
	}
}

// Resolve the conditionals `|` and `-` into precise instructions.
void resolve_conditionals(ParserError* err, strview src, PgnFunction* fn) {
	SVec2 pos = {0};
	for (pos.y = 0; pos.y < 256; pos.y++) {
		for (pos.x = 0; pos.x < 256; pos.x++) {
			Instruction* inst = get_instruction(fn, pos);
			if (*inst == I_COND_VT || *inst == I_COND_HZ) {
				// I_COND_HZ is right after I_COND_VT
				// D_UP is 0, then D_RIGHT, D_DOWN, D_LEFT
				Direction dir1 = *inst - I_COND_VT;
				Direction dir2 = dir1 + 2;
				Instruction adj1 = instruction_option(get_instruction(fn,
						go_direction(pos, dir1)), I_EMPTY);
				Instruction adj2 = instruction_option(get_instruction(fn,
						go_direction(pos, dir2)), I_EMPTY);
				if (adj1 && adj2) {
					*inst = I_COND_VT + dir1;
				}
				else if (adj1) {
					*inst = I_UP + dir1;
				}
				else if (adj2) {
					*inst = I_UP + dir2;
				}
				else {
					*inst = I_HALT_VT + dir1;
				}
			}
		}
	}
}
