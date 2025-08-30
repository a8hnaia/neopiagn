#ifndef PIAGN_PARSER_H
#define PIAGN_PARSER_H

#include "common.h"

typedef enum {
	PARSER_ERROR_NONE = 0,
	PARSER_ERROR_INVALID_INSTRUCTION,
	PARSER_ERROR_IMPROPER_BRACE,
	PARSER_ERROR_INVALID_START
} ParserErrorKind;

// Information about a parser error.
typedef struct {
	ParserErrorKind kind;
	SVec2 position;
	size_t index;
} ParserError;

// TODO
strview get_parser_error_message(ParserError err, strview src, strview* dest);

// Parses a piagn source file.
void parse_source(ParserError* err, strview src, PgnFunctions fns);

#endif
