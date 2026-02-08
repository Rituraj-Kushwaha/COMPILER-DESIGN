#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PROD 26
#define MAX_ALTS 32
#define MAX_TOKENS 32
#define MAX_TOKEN_LEN 16
#define MAX_TERMS 64
#define MAX_CELL 64

typedef struct {
	int token_count;
	char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
	char rhs_text[MAX_CELL];
} Alternative;

typedef struct {
	char lhs;
	int alt_count;
	Alternative alts[MAX_ALTS];
} Production;

static Production prods[MAX_PROD];
static int prod_count = 0;

static char nt_list[MAX_PROD];
static int nt_count = 0;

static char term_list[MAX_TERMS][MAX_TOKEN_LEN];
static int term_count = 0;

static bool first_set[MAX_PROD][MAX_TERMS];
static bool first_eps[MAX_PROD];
static bool follow_set[MAX_PROD][MAX_TERMS];

static char table[MAX_PROD][MAX_TERMS][MAX_CELL];
static bool has_conflict = false;

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

static int nt_index(char nt) {
	for (int i = 0; i < nt_count; i++) {
		if (nt_list[i] == nt) {
			return i;
		}
	}
	if (nt_count < MAX_PROD) {
		nt_list[nt_count] = nt;
		return nt_count++;
	}
	return -1;
}

static int term_index(const char *t) {
	for (int i = 0; i < term_count; i++) {
		if (strcmp(term_list[i], t) == 0) {
			return i;
		}
	}
	if (term_count < MAX_TERMS) {
		strncpy(term_list[term_count], t, MAX_TOKEN_LEN - 1);
		term_list[term_count][MAX_TOKEN_LEN - 1] = '\0';
		return term_count++;
	}
	return -1;
}

static int find_prod(char lhs) {
	for (int i = 0; i < prod_count; i++) {
		if (prods[i].lhs == lhs) {
			return i;
		}
	}
	return -1;
}

static void add_token(Alternative *alt, const char *tok) {
	if (alt->token_count < MAX_TOKENS) {
		strncpy(alt->tokens[alt->token_count], tok, MAX_TOKEN_LEN - 1);
		alt->tokens[alt->token_count][MAX_TOKEN_LEN - 1] = '\0';
		alt->token_count++;
	}
}

static void tokenize_rhs(const char *rhs, Alternative *alt) {
	alt->token_count = 0;
	strncpy(alt->rhs_text, rhs, MAX_CELL - 1);
	alt->rhs_text[MAX_CELL - 1] = '\0';

	if (strcmp(rhs, "eps") == 0) {
		add_token(alt, "eps");
		return;
	}

	int i = 0;
	while (rhs[i] != '\0') {
		if (isupper((unsigned char)rhs[i])) {
			char tok[2] = { rhs[i], '\0' };
			add_token(alt, tok);
			nt_index(rhs[i]);
			i++;
		} else if (islower((unsigned char)rhs[i]) || isdigit((unsigned char)rhs[i])) {
			char tok[MAX_TOKEN_LEN];
			int k = 0;
			while ((islower((unsigned char)rhs[i]) || isdigit((unsigned char)rhs[i])) && k < MAX_TOKEN_LEN - 1) {
				tok[k++] = rhs[i++];
			}
			tok[k] = '\0';
			add_token(alt, tok);
		} else {
			char tok[2] = { rhs[i], '\0' };
			add_token(alt, tok);
			i++;
		}
	}
}

static void parse_line(char *line) {
	trim_spaces(line);
	if (strlen(line) < 4) {
		return;
	}
	char lhs = line[0];
	if (!isupper((unsigned char)lhs)) {
		return;
	}
	char *arrow = strstr(line, "->");
	if (!arrow) {
		return;
	}
	int pidx = find_prod(lhs);
	if (pidx == -1) {
		pidx = prod_count++;
		prods[pidx].lhs = lhs;
		prods[pidx].alt_count = 0;
		nt_index(lhs);
	}

	char *rhs = arrow + 2;
	char *token = strtok(rhs, "|");
	while (token) {
		if (prods[pidx].alt_count < MAX_ALTS) {
			tokenize_rhs(token, &prods[pidx].alts[prods[pidx].alt_count]);
			prods[pidx].alt_count++;
		}
		token = strtok(NULL, "|");
	}
}

static bool is_nonterminal(const char *tok) {
	return strlen(tok) == 1 && isupper((unsigned char)tok[0]);
}

static bool is_epsilon(const char *tok) {
	return strcmp(tok, "eps") == 0;
}

static void collect_terminals(void) {
	for (int i = 0; i < prod_count; i++) {
		for (int j = 0; j < prods[i].alt_count; j++) {
			Alternative *alt = &prods[i].alts[j];
			for (int k = 0; k < alt->token_count; k++) {
				if (is_nonterminal(alt->tokens[k]) || is_epsilon(alt->tokens[k])) {
					continue;
				}
				term_index(alt->tokens[k]);
			}
		}
	}
	term_index("$");
}

static bool add_first(int A, int t) {
	if (!first_set[A][t]) {
		first_set[A][t] = true;
		return true;
	}
	return false;
}

static bool add_follow(int A, int t) {
	if (!follow_set[A][t]) {
		follow_set[A][t] = true;
		return true;
	}
	return false;
}

static bool compute_first(void) {
	bool changed = false;
	for (int i = 0; i < prod_count; i++) {
		int A = nt_index(prods[i].lhs);
		for (int j = 0; j < prods[i].alt_count; j++) {
			Alternative *alt = &prods[i].alts[j];
			bool all_eps = true;

			for (int k = 0; k < alt->token_count; k++) {
				const char *tok = alt->tokens[k];
				if (is_epsilon(tok)) {
					if (!first_eps[A]) {
						first_eps[A] = true;
						changed = true;
					}
					all_eps = false;
					break;
				} else if (!is_nonterminal(tok)) {
					int t = term_index(tok);
					if (add_first(A, t)) {
						changed = true;
					}
					all_eps = false;
					break;
				} else {
					int B = nt_index(tok[0]);
					for (int t = 0; t < term_count; t++) {
						if (first_set[B][t] && add_first(A, t)) {
							changed = true;
						}
					}
					if (!first_eps[B]) {
						all_eps = false;
						break;
					}
				}
			}

			if (all_eps) {
				if (!first_eps[A]) {
					first_eps[A] = true;
					changed = true;
				}
			}
		}
	}
	return changed;
}

static void first_of_sequence(Alternative *alt, int start, bool *out_first, bool *out_eps) {
	bool all_eps = true;
	for (int k = start; k < alt->token_count; k++) {
		const char *tok = alt->tokens[k];
		if (is_epsilon(tok)) {
			*out_eps = true;
			all_eps = false;
			break;
		} else if (!is_nonterminal(tok)) {
			int t = term_index(tok);
			out_first[t] = true;
			all_eps = false;
			break;
		} else {
			int B = nt_index(tok[0]);
			for (int t = 0; t < term_count; t++) {
				if (first_set[B][t]) {
					out_first[t] = true;
				}
			}
			if (!first_eps[B]) {
				all_eps = false;
				break;
			}
		}
	}
	if (all_eps) {
		*out_eps = true;
	}
}

static bool compute_follow(void) {
	bool changed = false;
	int dollar = term_index("$");
	int start_nt = nt_index(prods[0].lhs);
	if (add_follow(start_nt, dollar)) {
		changed = true;
	}

	for (int i = 0; i < prod_count; i++) {
		int A = nt_index(prods[i].lhs);
		for (int j = 0; j < prods[i].alt_count; j++) {
			Alternative *alt = &prods[i].alts[j];
			for (int k = 0; k < alt->token_count; k++) {
				const char *tok = alt->tokens[k];
				if (!is_nonterminal(tok)) {
					continue;
				}

				int B = nt_index(tok[0]);
				bool seq_first[MAX_TERMS] = {0};
				bool seq_eps = false;
				first_of_sequence(alt, k + 1, seq_first, &seq_eps);

				for (int t = 0; t < term_count; t++) {
					if (seq_first[t] && t != dollar) {
						if (add_follow(B, t)) {
							changed = true;
						}
					}
				}
				if (seq_eps || k == alt->token_count - 1) {
					for (int t = 0; t < term_count; t++) {
						if (follow_set[A][t] && add_follow(B, t)) {
							changed = true;
						}
					}
				}
			}
		}
	}
	return changed;
}

static void set_table(int A, int t, const char *prod_text) {
	if (table[A][t][0] == '\0') {
		strncpy(table[A][t], prod_text, MAX_CELL - 1);
		table[A][t][MAX_CELL - 1] = '\0';
	} else if (strcmp(table[A][t], prod_text) != 0) {
		strncpy(table[A][t], "conflict", MAX_CELL - 1);
		table[A][t][MAX_CELL - 1] = '\0';
		has_conflict = true;
	}
}

static void build_table(void) {
	for (int i = 0; i < nt_count; i++) {
		for (int t = 0; t < term_count; t++) {
			table[i][t][0] = '\0';
		}
	}

	for (int i = 0; i < prod_count; i++) {
		int A = nt_index(prods[i].lhs);
		for (int j = 0; j < prods[i].alt_count; j++) {
			Alternative *alt = &prods[i].alts[j];
			bool first_alt[MAX_TERMS] = {0};
			bool first_eps_alt = false;
			first_of_sequence(alt, 0, first_alt, &first_eps_alt);

			char prod_text[MAX_CELL];
			snprintf(prod_text, sizeof(prod_text), "%c->%s", prods[i].lhs, alt->rhs_text);

			for (int t = 0; t < term_count; t++) {
				if (first_alt[t]) {
					set_table(A, t, prod_text);
				}
			}
			if (first_eps_alt) {
				for (int t = 0; t < term_count; t++) {
					if (follow_set[A][t]) {
						set_table(A, t, prod_text);
					}
				}
			}
		}
	}
}

static void print_table(void) {
	printf("Predictive Parsing Table\n");
	printf("%-6s", "");
	for (int t = 0; t < term_count; t++) {
		printf(" %-12s", term_list[t]);
	}
	printf("\n");

	for (int i = 0; i < nt_count; i++) {
		printf("%-6c", nt_list[i]);
		for (int t = 0; t < term_count; t++) {
			if (table[i][t][0] == '\0') {
				printf(" %-12s", "-");
			} else {
				printf(" %-12s", table[i][t]);
			}
		}
		printf("\n");
	}
	printf("\n");

	if (has_conflict) {
		printf("Note: 'conflict' means the grammar is not LL(1).\n");
	}
}

int main(void) {
	int n = 0;
	char line[256];

	printf("Enter number of productions: ");
	if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_PROD) {
		return 1;
	}
	fgets(line, sizeof(line), stdin);

	for (int i = 0; i < n; i++) {
		printf("Enter production %d (e.g., E->TX): ", i + 1);
		if (!fgets(line, sizeof(line), stdin)) {
			return 1;
		}
		parse_line(line);
	}

	collect_terminals();

	bool changed = true;
	while (changed) {
		changed = compute_first();
	}

	changed = true;
	while (changed) {
		changed = compute_follow();
	}

	build_table();
	print_table();

	return 0;
}
