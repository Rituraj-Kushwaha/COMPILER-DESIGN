#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PROD 26
#define MAX_ALTS 32
#define MAX_LEN 64

typedef struct {
	char lhs;
	int alt_count;
	char alts[MAX_ALTS][MAX_LEN];
} Production;

static Production prods[MAX_PROD];
static int prod_count = 0;
static bool used_nt[26];

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

static int find_prod(char lhs) {
	for (int i = 0; i < prod_count; i++) {
		if (prods[i].lhs == lhs) {
			return i;
		}
	}
	return -1;
}

static char next_nonterminal(void) {
	for (int i = 0; i < 26; i++) {
		if (!used_nt[i]) {
			used_nt[i] = true;
			return (char)('A' + i);
		}
	}
	return '?';
}

static void add_production(char lhs, const char *alt) {
	int idx = find_prod(lhs);
	if (idx == -1) {
		if (prod_count >= MAX_PROD) {
			return;
		}
		idx = prod_count++;
		prods[idx].lhs = lhs;
		prods[idx].alt_count = 0;
		used_nt[lhs - 'A'] = true;
	}
	if (prods[idx].alt_count < MAX_ALTS) {
		strncpy(prods[idx].alts[prods[idx].alt_count], alt, MAX_LEN - 1);
		prods[idx].alts[prods[idx].alt_count][MAX_LEN - 1] = '\0';
		prods[idx].alt_count++;
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
	char *rhs = arrow + 2;
	char *token = strtok(rhs, "|");
	while (token) {
		add_production(lhs, token);
		token = strtok(NULL, "|");
	}
}

static void print_grammar(const char *title) {
	printf("%s\n", title);
	for (int i = 0; i < prod_count; i++) {
		printf("%c -> ", prods[i].lhs);
		for (int j = 0; j < prods[i].alt_count; j++) {
			printf("%s", prods[i].alts[j]);
			if (j + 1 < prods[i].alt_count) {
				printf(" | ");
			}
		}
		printf("\n");
	}
	printf("\n");
}

static void eliminate_left_recursion(void) {
	int original_count = prod_count;
	for (int i = 0; i < original_count; i++) {
		Production *p = &prods[i];
		char alpha[MAX_ALTS][MAX_LEN];
		char beta[MAX_ALTS][MAX_LEN];
		int alpha_count = 0;
		int beta_count = 0;

		for (int j = 0; j < p->alt_count; j++) {
			char *alt = p->alts[j];
			if (alt[0] == p->lhs) {
				strncpy(alpha[alpha_count], alt + 1, MAX_LEN - 1);
				alpha[alpha_count][MAX_LEN - 1] = '\0';
				alpha_count++;
			} else {
				strncpy(beta[beta_count], alt, MAX_LEN - 1);
				beta[beta_count][MAX_LEN - 1] = '\0';
				beta_count++;
			}
		}

		if (alpha_count == 0) {
			continue;
		}

		char new_nt = next_nonterminal();
		p->alt_count = 0;
		for (int j = 0; j < beta_count; j++) {
			char buf[MAX_LEN];
			snprintf(buf, sizeof(buf), "%s%c", beta[j], new_nt);
			add_production(p->lhs, buf);
		}

		for (int j = 0; j < alpha_count; j++) {
			char buf[MAX_LEN];
			snprintf(buf, sizeof(buf), "%s%c", alpha[j], new_nt);
			add_production(new_nt, buf);
		}
		add_production(new_nt, "eps");
	}
}

static int common_prefix_len(const char *a, const char *b) {
	int i = 0;
	while (a[i] && b[i] && a[i] == b[i]) {
		i++;
	}
	return i;
}

static bool left_factor_one(Production *p) {
	int best_len = 0;
	char best_prefix[MAX_LEN] = {0};

	for (int i = 0; i < p->alt_count; i++) {
		for (int j = i + 1; j < p->alt_count; j++) {
			int len = common_prefix_len(p->alts[i], p->alts[j]);
			if (len > best_len) {
				best_len = len;
				strncpy(best_prefix, p->alts[i], len);
				best_prefix[len] = '\0';
			}
		}
	}

	if (best_len == 0) {
		return false;
	}

	char new_nt = next_nonterminal();
	char new_alts[MAX_ALTS][MAX_LEN];
	int new_alt_count = 0;
	char factored_suffixes[MAX_ALTS][MAX_LEN];
	int suffix_count = 0;

	for (int i = 0; i < p->alt_count; i++) {
		if (strncmp(p->alts[i], best_prefix, best_len) == 0) {
			const char *suffix = p->alts[i] + best_len;
			if (*suffix == '\0') {
				strncpy(factored_suffixes[suffix_count++], "eps", MAX_LEN - 1);
			} else {
				strncpy(factored_suffixes[suffix_count++], suffix, MAX_LEN - 1);
			}
		} else {
			strncpy(new_alts[new_alt_count++], p->alts[i], MAX_LEN - 1);
		}
	}

	char factored[MAX_LEN];
	snprintf(factored, sizeof(factored), "%s%c", best_prefix, new_nt);
	strncpy(new_alts[new_alt_count++], factored, MAX_LEN - 1);

	p->alt_count = 0;
	for (int i = 0; i < new_alt_count; i++) {
		add_production(p->lhs, new_alts[i]);
	}

	for (int i = 0; i < suffix_count; i++) {
		add_production(new_nt, factored_suffixes[i]);
	}

	return true;
}

static void left_factoring(void) {
	for (int i = 0; i < prod_count; i++) {
		while (left_factor_one(&prods[i])) {
		}
	}
}

int main(void) {
	int n = 0;
	char line[256];

	printf("Ambiguity Example (Expression Grammar)\n");
	printf("Ambiguous: E -> E+E | E*E | (E) | id\n");
	printf("Unambiguous:\n");
	printf("E -> T E'\n");
	printf("E' -> + T E' | eps\n");
	printf("T -> F T'\n");
	printf("T' -> * F T' | eps\n");
	printf("F -> (E) | id\n\n");

	printf("Enter number of productions: ");
	if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_PROD) {
		return 1;
	}
	fgets(line, sizeof(line), stdin);

	for (int i = 0; i < n; i++) {
		printf("Enter production %d (e.g., A->Aa|b): ", i + 1);
		if (!fgets(line, sizeof(line), stdin)) {
			return 1;
		}
		parse_line(line);
	}

	print_grammar("Input Grammar");
	eliminate_left_recursion();
	print_grammar("After Left Recursion Elimination");
	left_factoring();
	print_grammar("After Left Factoring");

	return 0;
}
