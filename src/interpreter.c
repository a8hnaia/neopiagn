#include "interpreter.h"
#include <stdio.h>

ProgramState init_program() {
	ProgramState state = {0};
	state.stack.ptr = calloc(1, 256 * sizeof(uint8_t));
	assert(state.stack.ptr);
	state.piles = calloc(1, 256 * sizeof(Stack));
	assert(state.piles);
	for (size_t i = 0; i < 256; i++) {
		state.piles[i].ptr = calloc(1, 256 * sizeof(uint8_t));
		assert(state.piles[i].ptr);
	}
	return state;
}

void free_program(ProgramState* state) {
	free(state->stack.ptr);
	state->stack.ptr = NULL;
	for (size_t i = 0; i < 256; i++) {
		free(state->piles[i].ptr);
	}
	free(state->piles);
	state->piles = NULL;
}

void extend_stack(Stack* stack) {
	stack->capacity += 256;
	stack->ptr = realloc(stack->ptr, stack->capacity);
	assert(stack->ptr);
}

void push(Stack* stack, uint8_t item) {
	if (stack->length > stack->capacity) {
		extend_stack(stack);
	}
	stack->ptr[stack->length] = item;
	stack->length++;
}

uint8_t pop(Stack* stack) {
	if (stack->length == stack->start) {
		return 0;
	}
	else {
		stack->length--;
		return stack->ptr[stack->length];
	}
}

static void run_function(ProgramState* state, PgnFunctions fns, Instruction inst);

void run_instruction(ProgramState* state, PgnFunctions fns, Instruction inst) {
	switch (inst) {
		case I_EMPTY: break;
		case I_0:
			push(&state->stack, 0);
			break;
		case I_1: case I_2: case I_3: case I_4:
		case I_5: case I_6: case I_7: case I_8:
			push(&state->stack, 1 << (inst - I_1));
			break;
		case I_9:
			push(&state->stack, 255);
			break;
		case I_UP: case I_RIGHT: case I_DOWN: case I_LEFT:
			if (state->dir != (inst - I_UP + 2) % 4) {
				state->dir = inst - I_UP;
			}
			break;
		case I_COND_VT: case I_COND_HZ: {
			Direction dir1 = inst - I_COND_VT;
			Direction dir2 = dir1 + 2;
			if (state->dir != dir1 && state->dir != dir2) {
				if (pop(&state->stack)) {
					state->dir = (state->dir - 1) % 4;
				}
				else {
					state->dir = (state->dir + 1) % 4;
				}
			}
		} break;
		case I_HALT_VT: case I_HALT_HZ: {
			Direction dir1 = inst - I_HALT_VT;
			Direction dir2 = dir1 + 2;
			if (state->dir != dir1 && state->dir != dir2) {
				state->halt = true;
			}
		} break;
		case I_COND_TRI: {
			uint8_t top = pop(&state->stack);
			if (top == 255) {
				state->dir = (state->dir - 1) % 4;
			}
			else if (top == 0) {
				state->dir = (state->dir + 1) % 4;
			}
		} break;
		case I_MIRROR_RIGHT:
			state->dir ^= 1;
			break;
		case I_MIRROR_LEFT:
			state->dir ^= 3;
			break;
		case I_REPEAT:
			state->repeat = pop(&state->stack);
			break;
		case I_RANDOM:
			push(&state->stack, rand() % 256);
			break;
		case I_DUPLICATE: {
			uint8_t top = pop(&state->stack);
			push(&state->stack, top);
			push(&state->stack, top);
		} break;
		case I_SWAP: {
			uint8_t top = pop(&state->stack);
			uint8_t below = pop(&state->stack);
			push(&state->stack, top);
			push(&state->stack, below);
		} break;
		case I_DROP:
			pop(&state->stack);
			break;
		case I_XOR:
			push(&state->stack, pop(&state->stack) ^ pop(&state->stack));
			break;
		case I_AND:
			push(&state->stack, pop(&state->stack) & pop(&state->stack));
			break;
		case I_OR:
			push(&state->stack, pop(&state->stack) | pop(&state->stack));
			break;
		case I_SHIFT_LEFT:
			push(&state->stack, pop(&state->stack) << 1);
			break;
		case I_SHIFT_RIGHT:
			push(&state->stack, pop(&state->stack) >> 1);
			break;
		case I_NOT:
			push(&state->stack, ~pop(&state->stack));
			break;
		case I_GO_PILE:
			state->pile_index = pop(&state->stack);
			break;
		case I_LEFT_PILE:
			state->pile_index = (state->pile_index - 1) % 256;
			break;
		case I_RIGHT_PILE:
			state->pile_index = (state->pile_index + 1) % 256;
			break;
		case I_FROM_PILE:
			push(&state->stack,
					pop(&state->piles[state->pile_index]));
			break;
		case I_TO_PILE:
			push(&state->piles[state->pile_index],
					pop(&state->stack));
			break;
		case I_READ:
			push(&state->stack, fgetc(stdin));
			break;
		case I_WRITE:
			fputc(pop(&state->stack), stdout);
			fflush(stdout);
			break;
		default: {
			Direction dir = state->dir;
			size_t pile_lengths[256];
			size_t pile_starts[256];
			for (int i = 0; i < 256; i++) {
				pile_lengths[i] = state->piles[i].length;
				pile_starts[i] = state->piles[i].start;
				state->piles[i].start = state->piles[i].length;
			}
			run_function(state, fns, inst);
			for (int i = 0; i < 256; i++) {
				state->piles[i].length = pile_lengths[i];
				state->piles[i].start = pile_starts[i];
			}
			state->dir = dir;
		} break;
	}
}

void run_function(ProgramState* state, PgnFunctions fns, Instruction inst) {
	PgnFunction* fn = get_function(fns, inst);
	SVec2 pos = fn->start_position;
	Direction dir = fn->start_direction;
	uint8_t repeat = 0;
	size_t pile_index = 0;
	for (;;) {
		if (pos.x >= 256 * fn->size.x || pos.y >= 256 * fn->size.y) {
			return;
		}
		state->dir = dir;
		state->pile_index = pile_index;
		Instruction inst = *get_instruction(fn, pos);
		if (repeat) {
			switch (inst) {
				case I_EMPTY:
				case I_UP: case I_RIGHT: case I_DOWN: case I_LEFT:
				case I_COND_VT: case I_COND_HZ: case I_HALT_VT: case I_HALT_HZ:
				case I_COND_TRI: case I_MIRROR_RIGHT: case I_MIRROR_LEFT:
					run_instruction(state, fns, inst);
					break;
				default:
					for (uint8_t i = 0; i < repeat; i++) {
						run_instruction(state, fns, inst);
					}
					repeat = 0;
					break;
			}
		}
		else {
			run_instruction(state, fns, inst);
		}
		if (state->halt) {
			state->halt = false;
			return;
		}
		dir = state->dir;
		pile_index = state->pile_index;
		pos = go_direction(pos, dir);
		repeat = state->repeat;
		state->repeat = 0;
	}
}

void run_program(ProgramState* state, PgnFunctions fns) {
	run_function(state, fns, I_MAIN);
	state->stack.length = 0;
	for (int i = 0; i < 256; i++) {
		state->piles[i].length = 0;
	}
}
