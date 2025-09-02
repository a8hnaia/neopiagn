#include "common.h"

PgnFunctions init_functions() {
	PgnFunctions fns = {0};
	// Lowercase and uppercase letters, plus main
	fns.ptr = calloc(1, 53 * sizeof(PgnFunction));
	assert(fns.ptr);
	for (int i = 0; i < 53; i++) {
		fns.ptr[i].size = (SVec2){.x = 1, .y = 1};
		fns.ptr[i].grids = calloc(1, sizeof(Instruction*));
		assert(fns.ptr[i].grids);
		fns.ptr[i].grids[0] = calloc(1, 256 * 256 * sizeof(Instruction));
		assert(fns.ptr[i].grids[0]);
	}
	return fns;
}

void free_functions(PgnFunctions* fns) {
	for (size_t i = 0; i < 53; i++) {
		for (size_t j = 0; j < fns->ptr[i].size.x; j++) {
			free(fns->ptr[i].grids[j]);
		}
		free(fns->ptr[i].grids);
	}
	free(fns->ptr);
	fns->ptr = 0;
};

PgnFunction* get_function(PgnFunctions fns, Instruction name) {
	if (I_UPPER_A <= name && name <= I_MAIN) {
		return &fns.ptr[name - I_UPPER_A];
	}
	else {
		return NULL;
	}
}

Instruction* get_instruction(PgnFunction* fn, SVec2 pos) {
	if (pos.x > 256 * fn->size.x || pos.y > 256 * fn->size.y) {
		return NULL;
	}
	else {
		return &fn->grids[pos.x / 256][256 * (pos.y % 256) + pos.x];
	}
}

void accomodate_position(PgnFunction* fn, SVec2 pos) {
	if (pos.y > 256 * fn->size.y) {
		size_t old_height = fn->size.y;
		fn->size.y = pos.y / 256 + 1;
		for (size_t i = 0; i < fn->size.x; i++) {
			fn->grids[i] = realloc(fn->grids[i],
					fn->size.y * 256 * 256 * sizeof(uint8_t));
			assert(fn->grids[i]);
			memset(fn->grids[i] + 256 * 256 * old_height, 0,
					256 * 256 * (fn->size.y - old_height));
		}
	}
	if (pos.x > 256 * fn->size.x) {
		size_t old_width = fn->size.x;
		fn->size.x = pos.x / 256 + 1;
		fn->grids = realloc(fn->grids, fn->size.x * sizeof(Instruction*));
		assert(fn->grids);
		for (size_t i = old_width; i < fn->size.x; i++) {
			fn->grids[i] = calloc(1, fn->size.y * 256 * 256 * sizeof(uint8_t));
			assert(fn->grids[i]);
		}
	}
}

SVec2 go_direction(SVec2 pos, Direction dir) {
	SVec2 new_pos = pos;
	switch (dir) {
		case D_UP:
			new_pos.y--;
			break;
		case D_RIGHT:
			new_pos.x++;
			break;
		case D_DOWN:
			new_pos.y++;
			break;
		case D_LEFT:
			new_pos.x--;
			break;
	}
	return new_pos;
}

Instruction instruction_option(Instruction* inst, Instruction fallback) {
	if (inst) {
		return *inst;
	}
	else {
		return fallback;
	}
}
