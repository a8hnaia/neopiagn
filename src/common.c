#include "common.h"

PgnFunctions init_functions() {
	PgnFunctions fns = {0};
	// Lowercase and uppercase letters, plus main
	void* ptr = calloc(1, 53 * sizeof(PgnFunction)
			+ 53 * 256 * 256 * sizeof(Instruction));
	fns.ptr = ptr;
	ptr += 53 * sizeof(PgnFunction);
	for (int i = 0; i < 53; i++) {
		fns.ptr[i].grid = ptr;
		ptr += 256 * 256 * sizeof(Instruction);
	}
	return fns;
}

void free_functions(PgnFunctions* fns) {
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
	if (pos.x > 255 || pos.y > 255) {
		return NULL;
	}
	else {
		return &fn->grid[256 * pos.y + pos.x];
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
