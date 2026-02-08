#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BLOCKS 64
#define HEAP_SIZE 1024

typedef struct {
	int start;
	int size;
	int id;
	bool free;
} Block;

static Block blocks[MAX_BLOCKS];
static int block_count = 0;

static void init_heap(void) {
	block_count = 1;
	blocks[0].start = 0;
	blocks[0].size = HEAP_SIZE;
	blocks[0].id = -1;
	blocks[0].free = true;
}

static void print_heap(void) {
	printf("\nHeap Blocks (First-Fit)\n");
	printf("%-6s %-6s %-6s %-6s\n", "Idx", "Start", "Size", "Id");
	for (int i = 0; i < block_count; i++) {
		printf("%-6d %-6d %-6d ", i, blocks[i].start, blocks[i].size);
		if (blocks[i].free) {
			printf("free\n");
		} else {
			printf("%d\n", blocks[i].id);
		}
	}
	printf("\n");
}

static int find_block_by_id(int id) {
	for (int i = 0; i < block_count; i++) {
		if (!blocks[i].free && blocks[i].id == id) {
			return i;
		}
	}
	return -1;
}

static void merge_free_blocks(void) {
	for (int i = 0; i < block_count - 1; ) {
		if (blocks[i].free && blocks[i + 1].free) {
			blocks[i].size += blocks[i + 1].size;
			for (int j = i + 1; j < block_count - 1; j++) {
				blocks[j] = blocks[j + 1];
			}
			block_count--;
		} else {
			i++;
		}
	}
}

static bool allocate_first_fit(int id, int size) {
	if (size <= 0) {
		return false;
	}
	if (find_block_by_id(id) != -1) {
		return false;
	}

	for (int i = 0; i < block_count; i++) {
		if (blocks[i].free && blocks[i].size >= size) {
			if (blocks[i].size == size) {
				blocks[i].free = false;
				blocks[i].id = id;
			} else {
				if (block_count >= MAX_BLOCKS) {
					return false;
				}
				Block new_block;
				new_block.start = blocks[i].start;
				new_block.size = size;
				new_block.id = id;
				new_block.free = false;

				blocks[i].start += size;
				blocks[i].size -= size;

				for (int j = block_count; j > i; j--) {
					blocks[j] = blocks[j - 1];
				}
				blocks[i] = new_block;
				block_count++;
			}
			return true;
		}
	}
	return false;
}

static bool free_block(int id) {
	int idx = find_block_by_id(id);
	if (idx == -1) {
		return false;
	}
	blocks[idx].free = true;
	blocks[idx].id = -1;
	merge_free_blocks();
	return true;
}

int main(void) {
	char cmd[16];
	init_heap();

	printf("Heap Allocation (First-Fit)\n");
	printf("Commands:\n");
	printf("  alloc <id> <size>\n");
	printf("  free <id>\n");
	printf("  print\n");
	printf("  exit\n\n");

	while (1) {
		printf("> ");
		if (scanf("%15s", cmd) != 1) {
			return 0;
		}
		if (strcmp(cmd, "alloc") == 0) {
			int id = 0;
			int size = 0;
			scanf("%d %d", &id, &size);
			if (allocate_first_fit(id, size)) {
				printf("Allocated id=%d size=%d\n", id, size);
			} else {
				printf("Allocation failed\n");
			}
		} else if (strcmp(cmd, "free") == 0) {
			int id = 0;
			scanf("%d", &id);
			if (free_block(id)) {
				printf("Freed id=%d\n", id);
			} else {
				printf("Free failed\n");
			}
		} else if (strcmp(cmd, "print") == 0) {
			print_heap();
		} else if (strcmp(cmd, "exit") == 0) {
			break;
		} else {
			printf("Unknown command\n");
		}
	}

	printf("\nSample input:\n");
	printf("alloc 1 100\nalloc 2 200\nprint\nfree 1\nalloc 3 50\nprint\nexit\n");
	return 0;
}
