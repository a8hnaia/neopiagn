#ifndef PIAGN_INTERPRETER_H
#define PIAGN_INTERPRETER_H

#include "common.h"

// Linked list of stack continuations.
typedef struct StackContinuation {
	uint8_t* ptr;
	struct StackContinuation* next;
} StackContinuation;

// The stack or a pile.
typedef struct {
	uint8_t* ptr;
	size_t length;
	size_t start;
	StackContinuation* next;
} Stack;

// Data for a piagn program to run.
typedef struct {
	bool halt;
	uint8_t repeat;
	size_t pile_index;
	SVec2 pos;
	Direction dir;
	Stack stack;
	Stack* piles;
} ProgramState;

// Allocates and initializes a ProgramState.
ProgramState init_program();

// Frees the ProgramState.
void free_program(ProgramState* state);

// Runs a piagn program.
void run_program(ProgramState* state, PgnFunctions fns);

#endif
