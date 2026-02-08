#include <stdio.h>
#include <stdbool.h>

#define MAX_STATES 8
#define MAX_SYMBOLS 2
#define MAX_DFA_STATES 32

typedef unsigned long long Bitset;

static Bitset nfa_trans[MAX_STATES][MAX_SYMBOLS];
static Bitset nfa_eps[MAX_STATES];
static char symbols[MAX_SYMBOLS] = { 'a', 'b' };

static Bitset set_bit(int s) {
	return 1ULL << s;
}

static bool has_bit(Bitset s, int i) {
	return (s & set_bit(i)) != 0ULL;
}

static Bitset epsilon_closure(Bitset start_set, int n_states) {
	Bitset closure = start_set;
	int stack[MAX_STATES];
	int top = 0;

	for (int i = 0; i < n_states; i++) {
		if (has_bit(start_set, i)) {
			stack[top++] = i;
		}
	}

	while (top > 0) {
		int s = stack[--top];
		Bitset next = nfa_eps[s];
		for (int i = 0; i < n_states; i++) {
			if (has_bit(next, i) && !has_bit(closure, i)) {
				closure |= set_bit(i);
				stack[top++] = i;
			}
		}
	}

	return closure;
}

static Bitset move_on_symbol(Bitset set, int sym, int n_states) {
	Bitset result = 0ULL;
	for (int i = 0; i < n_states; i++) {
		if (has_bit(set, i)) {
			result |= nfa_trans[i][sym];
		}
	}
	return result;
}

static int find_dfa_state(Bitset *dfa_states, int dfa_count, Bitset set) {
	for (int i = 0; i < dfa_count; i++) {
		if (dfa_states[i] == set) {
			return i;
		}
	}
	return -1;
}

static void print_set(Bitset set, int n_states) {
	int first = 1;
	printf("{");
	for (int i = 0; i < n_states; i++) {
		if (has_bit(set, i)) {
			if (!first) {
				printf(",");
			}
			printf("%d", i);
			first = 0;
		}
	}
	printf("}");
}

static void load_sample_nfa(int *n_states, int *n_symbols, int *start_state, Bitset *final_mask) {
	*n_states = 3;
	*n_symbols = 2;
	*start_state = 0;
	*final_mask = set_bit(2);

	for (int s = 0; s < MAX_STATES; s++) {
		for (int a = 0; a < MAX_SYMBOLS; a++) {
			nfa_trans[s][a] = 0ULL;
		}
		nfa_eps[s] = 0ULL;
	}

	/* NFA that accepts strings ending with "ab" */
	nfa_trans[0][0] |= set_bit(1); /* 0 -a-> 1 */
	nfa_trans[0][1] |= set_bit(0); /* 0 -b-> 0 */
	nfa_trans[1][0] |= set_bit(1); /* 1 -a-> 1 */
	nfa_trans[1][1] |= set_bit(2); /* 1 -b-> 2 */
	nfa_trans[2][0] |= set_bit(1); /* 2 -a-> 1 */
	nfa_trans[2][1] |= set_bit(0); /* 2 -b-> 0 */
}

int main(void) {
	int n_states = 0;
	int n_symbols = 0;
	int start_state = 0;
	Bitset final_mask = 0ULL;

	load_sample_nfa(&n_states, &n_symbols, &start_state, &final_mask);

	Bitset dfa_states[MAX_DFA_STATES];
	int dfa_trans[MAX_DFA_STATES][MAX_SYMBOLS];
	int dfa_count = 0;

	Bitset start_set = epsilon_closure(set_bit(start_state), n_states);
	dfa_states[dfa_count++] = start_set;

	for (int i = 0; i < MAX_DFA_STATES; i++) {
		for (int a = 0; a < n_symbols; a++) {
			dfa_trans[i][a] = -1;
		}
	}

	int idx = 0;
	while (idx < dfa_count) {
		Bitset current = dfa_states[idx];
		for (int a = 0; a < n_symbols; a++) {
			Bitset moved = move_on_symbol(current, a, n_states);
			Bitset next = epsilon_closure(moved, n_states);
			if (next == 0ULL) {
				dfa_trans[idx][a] = -1;
				continue;
			}

			int existing = find_dfa_state(dfa_states, dfa_count, next);
			if (existing == -1) {
				if (dfa_count >= MAX_DFA_STATES) {
					return 1;
				}
				dfa_states[dfa_count] = next;
				dfa_trans[idx][a] = dfa_count;
				dfa_count++;
			} else {
				dfa_trans[idx][a] = existing;
			}
		}
		idx++;
	}

	printf("NFA to DFA Conversion\n");
	printf("NFA: accepts strings over {a,b} that end with \"ab\"\n\n");

	printf("DFA Transition Table\n");
	printf("%-8s %-16s", "State", "Set");
	for (int a = 0; a < n_symbols; a++) {
		printf(" %-8c", symbols[a]);
	}
	printf("\n");
	printf("------------------------------------------------------------\n");

	for (int i = 0; i < dfa_count; i++) {
		bool accepting = (dfa_states[i] & final_mask) != 0ULL;
		if (accepting) {
			printf("*D%-6d ", i);
		} else {
			printf("D%-7d ", i);
		}

		print_set(dfa_states[i], n_states);
		int pad = 16 - 2;
		printf("%*s", pad, "");

		for (int a = 0; a < n_symbols; a++) {
			if (dfa_trans[i][a] == -1) {
				printf(" %-8s", "-");
			} else {
				printf(" D%-7d", dfa_trans[i][a]);
			}
		}
		printf("\n");
	}

	printf("\nNote: *Dk means accepting DFA state.\n");
	return 0;
}
