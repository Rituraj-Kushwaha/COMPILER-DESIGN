#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BLOCKS 16
#define MAX_VARS 16
#define MAX_LINE 256

typedef struct {
	bool gen[MAX_VARS];
	bool kill[MAX_VARS];
	bool in[MAX_VARS];
	bool out[MAX_VARS];
	bool pred[MAX_BLOCKS];
} Block;

static Block blocks[MAX_BLOCKS];
static int block_count = 0;

static char vars[MAX_VARS];
static int var_count = 0;

static int var_index(char v) {
	for (int i = 0; i < var_count; i++) {
		if (vars[i] == v) {
			return i;
		}
	}
	if (var_count < MAX_VARS) {
		vars[var_count] = v;
		return var_count++;
	}
	return -1;
}

static void init_block(Block *b) {
	for (int i = 0; i < MAX_VARS; i++) {
		b->gen[i] = false;
		b->kill[i] = false;
		b->in[i] = false;
		b->out[i] = false;
	}
	for (int i = 0; i < MAX_BLOCKS; i++) {
		b->pred[i] = false;
	}
}

static void print_set(const char *name, bool set[MAX_VARS]) {
	printf("%s { ", name);
	bool first = true;
	for (int i = 0; i < var_count; i++) {
		if (set[i]) {
			if (!first) {
				printf(", ");
			}
			printf("%c", vars[i]);
			first = false;
		}
	}
	printf(" }\n");
}

static void compute_out(int b) {
	for (int i = 0; i < var_count; i++) {
		blocks[b].out[i] = blocks[b].gen[i] || (blocks[b].in[i] && !blocks[b].kill[i]);
	}
}

static bool update_in(int b) {
	bool changed = false;
	for (int v = 0; v < var_count; v++) {
		bool new_in = false;
		for (int p = 0; p < block_count; p++) {
			if (blocks[b].pred[p]) {
				if (blocks[p].out[v]) {
					new_in = true;
					break;
				}
			}
		}
		if (blocks[b].in[v] != new_in) {
			blocks[b].in[v] = new_in;
			changed = true;
		}
	}
	return changed;
}

int main(void) {
	printf("Global Data Flow Analysis: Reaching Definitions (bitset form)\n\n");

	for (int i = 0; i < MAX_BLOCKS; i++) {
		init_block(&blocks[i]);
	}

	block_count = 3;
	var_index('a');
	var_index('b');
	var_index('c');

	blocks[0].gen[var_index('a')] = true;
	blocks[0].kill[var_index('a')] = true;

	blocks[1].gen[var_index('b')] = true;
	blocks[1].kill[var_index('b')] = true;

	blocks[2].gen[var_index('a')] = true;
	blocks[2].kill[var_index('a')] = true;
	blocks[2].gen[var_index('c')] = true;
	blocks[2].kill[var_index('c')] = true;

	blocks[1].pred[0] = true;
	blocks[2].pred[1] = true;
	blocks[1].pred[2] = true;

	bool changed = true;
	while (changed) {
		changed = false;
		for (int b = 0; b < block_count; b++) {
			if (update_in(b)) {
				changed = true;
			}
			compute_out(b);
		}
	}

	for (int b = 0; b < block_count; b++) {
		printf("Block B%d\n", b);
		print_set("  GEN:", blocks[b].gen);
		print_set("  KILL:", blocks[b].kill);
		print_set("  IN:", blocks[b].in);
		print_set("  OUT:", blocks[b].out);
		printf("\n");
	}

	printf("Sample CFG used:\n");
	printf("B0: a = 1\n");
	printf("B1: b = a + 1\n");
	printf("B2: a = b + c; c = a + 1\n");
	printf("Edges: B0->B1, B1->B2, B2->B1\n");

	return 0;
}
