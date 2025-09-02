#ifndef PIAGN_COMMON_H
#define PIAGN_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// A string view.
typedef struct {
	char* ptr;
	size_t len;
} strview;

// Macro to pass a string view to printf.
#define sv_fmt(s) (s).ptr, (int)(s).len
// Format string for a string view.
#define PRI_SV "%.*s"
// Convenience macro to make a string view from a string literal.
#define lit_sv(c) (strview){.ptr=c,.len=strlen(c)}

// The column and line in a source file or grid.
typedef struct {
	 size_t x;
	 size_t y;
} SVec2;

// Cardinal directions.
typedef enum {
	D_UP = 0,
	D_RIGHT = 1,
	D_DOWN = 2,
	D_LEFT = 3
} Direction;

// Represents an instruction in piagn code.
typedef enum {
	// Empty (` ` or `@`)
	I_EMPTY = 0,
	// Number instructions
	I_0, I_1, I_2, I_3, I_4, I_5, I_6, I_7, I_8, I_9,
	// Directions (transformed from `|` and `-`)
	I_UP, I_RIGHT, I_DOWN, I_LEFT,
	// Conditionals (transformed from `|` and `-`)
	I_COND_VT, I_COND_HZ,
	// Halters (transformed from `|` and `-`)
	I_HALT_VT, I_HALT_HZ,
	// Three-way conditional (`*`)
	I_COND_TRI,
	// Mirrors (`/` and `\`)
	I_MIRROR_RIGHT, I_MIRROR_LEFT,
	// Stack manipulators (`#`, `?`, `!`, `%`, `_`)
	I_REPEAT, I_RANDOM, I_DUPLICATE, I_SWAP, I_DROP,
	// Bitwise operations (`^`, `&`, `+`, `[`, `]`, `~`)
	I_XOR, I_AND, I_OR, I_SHIFT_LEFT, I_SHIFT_RIGHT, I_NOT,
	// Piles operations (`$`, `<`, `>`, `,`, `.`)
	I_GO_PILE, I_LEFT_PILE, I_RIGHT_PILE, I_FROM_PILE, I_TO_PILE,
	// Input/Output (`;`, `:`)
	I_READ, I_WRITE,
	// Functions
	I_UPPER_A, I_UPPER_B, I_UPPER_C, I_UPPER_D, I_UPPER_E, I_UPPER_F,
	I_UPPER_G, I_UPPER_H, I_UPPER_I, I_UPPER_J, I_UPPER_K, I_UPPER_L,
	I_UPPER_M, I_UPPER_N, I_UPPER_O, I_UPPER_P, I_UPPER_Q, I_UPPER_R,
	I_UPPER_S, I_UPPER_T, I_UPPER_U, I_UPPER_V, I_UPPER_W, I_UPPER_X,
	I_UPPER_Y, I_UPPER_Z,
	I_LOWER_A, I_LOWER_B, I_LOWER_C, I_LOWER_D, I_LOWER_E, I_LOWER_F,
	I_LOWER_G, I_LOWER_H, I_LOWER_I, I_LOWER_J, I_LOWER_K, I_LOWER_L,
	I_LOWER_M, I_LOWER_N, I_LOWER_O, I_LOWER_P, I_LOWER_Q, I_LOWER_R,
	I_LOWER_S, I_LOWER_T, I_LOWER_U, I_LOWER_V, I_LOWER_W, I_LOWER_X,
	I_LOWER_Y, I_LOWER_Z,
	// Main function
	I_MAIN
} Instruction;

// Data necessary to call a piagn function.
typedef struct {
	Direction start_direction;
	SVec2 start_position;
	SVec2 size;
	Instruction** grids;
} PgnFunction;

// An array of 53 (lowercase/uppercase letters and main) piagn functions.
typedef struct {
	PgnFunction* ptr;
} PgnFunctions;

// Allocates and initializes a PgnFunctions.
PgnFunctions init_functions();

// Frees the PgnFunctions.
void free_functions(PgnFunctions* fns);

// Gets a pointer to a piagn function.
// Returns NULL if name is not a function.
PgnFunction* get_function(PgnFunctions fns, Instruction name);

// Gets an instruction from a piagn function.
// Returns NULL if pos is out of range.
Instruction* get_instruction(PgnFunction* fn, SVec2 pos);

// The position moved in the direction.
SVec2 go_direction(SVec2 pos, Direction dir);

// Expand function grid to fit position.
void accomodate_position(PgnFunction* fn, SVec2 pos);

// The Instruction pointed to if it's not NULL, otherwise fallback.
Instruction instruction_option(Instruction* inst, Instruction fallback);

#endif
