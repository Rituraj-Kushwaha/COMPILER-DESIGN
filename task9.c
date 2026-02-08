#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PROD 64
#define MAX_LEN 64
#define MAX_ITEMS 256
#define MAX_STATES 128
#define MAX_SYMBOLS 64

typedef struct {
	char lhs;
	char rhs[MAX_LEN];
} Production;

typedef struct {
	int prod;
	int dot;
} Item;

typedef struct {
	Item items[MAX_ITEMS];
	int count;
	int trans[MAX_SYMBOLS];
} State;

static Production prods[MAX_PROD];
static int prod_count = 0;

static char symbols[MAX_SYMBOLS];
static int sym_count = 0;

static State states[MAX_STATES];
static int state_count = 0;

static void trim_spaces(char *s) {
	char *p = s;
	char *q = s;
	while (*p) {
		if (!isspace((unsigned char)*p)) {
			*q++ = *p;
		}
		p++;
	}
	*q = '\0';
}

static bool is_nonterminal(char c) {
	return isupper((unsigned char)c);
}

static int sym_index(char c) {
	for (int i = 0; i < sym_count; i++) {
		if (symbols[i] == c) {
			return i;
		}
	}
	if (sym_count < MAX_SYMBOLS) {
		symbols[sym_count] = c;
		return sym_count++;
	}
	return -1;
}

static void add_production(char lhs, const char *rhs) {
	if (prod_count < MAX_PROD) {
		prods[prod_count].lhs = lhs;
		strncpy(prods[prod_count].rhs, rhs, MAX_LEN - 1);
		prods[prod_count].rhs[MAX_LEN - 1] = '\0';
		prod_count++;
	}
}

static void parse_line(char *line) {
	trim_spaces(line);
	if (strlen(line) < 4) {
		return;
	}
	char lhs = line[0];
	if (!is_nonterminal(lhs)) {
		return;
	}
	char *arrow = strstr(line, "->");
	if (!arrow) {
		return;
	}
	char *rhs = arrow + 2;
	char *token = strtok(rhs, "|");
	while (token) {
		add_production(lhs, token);
		token = strtok(NULL, "|");
	}
}

static void collect_symbols(void) {
	for (int i = 0; i < prod_count; i++) {
		sym_index(prods[i].lhs);
		for (int j = 0; prods[i].rhs[j] != '\0'; j++) {
			sym_index(prods[i].rhs[j]);
		}
	}
}

static bool item_exists(State *st, int prod, int dot) {
	for (int i = 0; i < st->count; i++) {
		if (st->items[i].prod == prod && st->items[i].dot == dot) {
			return true;
		}
	}
	return false;
}

static void add_item(State *st, int prod, int dot) {
	if (st->count >= MAX_ITEMS) {
		return;
	}
	if (!item_exists(st, prod, dot)) {
		st->items[st->count].prod = prod;
		st->items[st->count].dot = dot;
		st->count++;
	}
}

static void closure(State *st) {
	bool changed = true;
	while (changed) {
		changed = false;
		for (int i = 0; i < st->count; i++) {
			Item it = st->items[i];
			char *rhs = prods[it.prod].rhs;
			int len = (int)strlen(rhs);
			if (it.dot < len) {
				char sym = rhs[it.dot];
				if (is_nonterminal(sym)) {
					for (int p = 0; p < prod_count; p++) {
						if (prods[p].lhs == sym) {
							if (!item_exists(st, p, 0)) {
								add_item(st, p, 0);
								changed = true;
							}
						}
					}
				}
			}
		}
	}
}

static void sort_items(State *st) {
	for (int i = 0; i < st->count - 1; i++) {
		for (int j = i + 1; j < st->count; j++) {
			Item a = st->items[i];
			Item b = st->items[j];
			if (a.prod > b.prod || (a.prod == b.prod && a.dot > b.dot)) {
				st->items[i] = b;
				st->items[j] = a;
			}
		}
	}
}

static bool same_state(State *a, State *b) {
	if (a->count != b->count) {
		return false;
	}
	for (int i = 0; i < a->count; i++) {
		if (a->items[i].prod != b->items[i].prod || a->items[i].dot != b->items[i].dot) {
			return false;
		}
	}
	return true;
}

static int find_state(State *st) {
	for (int i = 0; i < state_count; i++) {
		if (same_state(&states[i], st)) {
			return i;
		}
	}
	return -1;
}

static State goto_state(State *st, char sym) {
	State next;
	next.count = 0;
	for (int i = 0; i < MAX_SYMBOLS; i++) {
		next.trans[i] = -1;
	}

	for (int i = 0; i < st->count; i++) {
		Item it = st->items[i];
		char *rhs = prods[it.prod].rhs;
		int len = (int)strlen(rhs);
		if (it.dot < len && rhs[it.dot] == sym) {
			add_item(&next, it.prod, it.dot + 1);
		}
	}

	closure(&next);
	sort_items(&next);
	return next;
}

static void build_canonical_collection(void) {
	State start;
	start.count = 0;
	for (int i = 0; i < MAX_SYMBOLS; i++) {
		start.trans[i] = -1;
	}

	add_item(&start, 0, 0);
	closure(&start);
	sort_items(&start);

	states[state_count++] = start;

	int idx = 0;
	while (idx < state_count) {
		State *cur = &states[idx];
		for (int s = 0; s < sym_count; s++) {
			State next = goto_state(cur, symbols[s]);
			if (next.count == 0) {
				continue;
			}
			int existing = find_state(&next);
			if (existing == -1) {
				if (state_count >= MAX_STATES) {
					return;
				}
				states[state_count] = next;
				cur->trans[s] = state_count;
				state_count++;
			} else {
				cur->trans[s] = existing;
			}
		}
		idx++;
	}
}

static void print_item(Item it) {
	char *rhs = prods[it.prod].rhs;
	int len = (int)strlen(rhs);
	printf("%c -> ", prods[it.prod].lhs);
	for (int i = 0; i <= len; i++) {
		if (i == it.dot) {
			printf(".");
		}
		if (i < len) {
			printf("%c", rhs[i]);
		}
	}
}

static void print_states(void) {
	for (int i = 0; i < state_count; i++) {
		printf("I%d:\n", i);
		for (int j = 0; j < states[i].count; j++) {
			printf("  ");
			print_item(states[i].items[j]);
			printf("\n");
		}
		printf("\n");
	}
}

static void print_goto_table(void) {
	printf("GOTO Table\n");
	printf("%-6s", "");
	for (int s = 0; s < sym_count; s++) {
		printf(" %-6c", symbols[s]);
	}
	printf("\n");

	for (int i = 0; i < state_count; i++) {
		printf("%-6d", i);
		for (int s = 0; s < sym_count; s++) {
			if (states[i].trans[s] == -1) {
				printf(" %-6s", "-");
			} else {
				printf(" %-6d", states[i].trans[s]);
			}
		}
		printf("\n");
	}
	printf("\n");
}

static char pick_augmented_start(void) {
	for (char c = 'Z'; c >= 'A'; c--) {
		bool used = false;
		for (int i = 0; i < prod_count; i++) {
			if (prods[i].lhs == c) {
				used = true;
				break;
			}
		}
		if (!used) {
			return c;
		}
	}
	return 'Z';
}

int main(void) {
	int n = 0;
	char line[256];

	printf("Enter number of productions: ");
	if (scanf("%d", &n) != 1 || n <= 0) {
		return 1;
	}
	fgets(line, sizeof(line), stdin);

	for (int i = 0; i < n; i++) {
		printf("Enter production %d (e.g., S->CC|c): ", i + 1);
		if (!fgets(line, sizeof(line), stdin)) {
			return 1;
		}
		parse_line(line);
	}

	if (prod_count == 0) {
		return 1;
	}

	char start_lhs = prods[0].lhs;
	char aug = pick_augmented_start();

	for (int i = prod_count; i > 0; i--) {
		prods[i] = prods[i - 1];
	}
	prod_count++;
	prods[0].lhs = aug;
	prods[0].rhs[0] = start_lhs;
	prods[0].rhs[1] = '\0';

	collect_symbols();
	build_canonical_collection();

	printf("\nLR(0) Items (Canonical Collection)\n\n");
	print_states();
	print_goto_table();

	printf("Sample input:\n");
	printf("2\nS->CC\nC->cC|d\n");

	return 0;
}
