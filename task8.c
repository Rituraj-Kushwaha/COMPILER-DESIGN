#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PROD 26
#define MAX_ALTS 32
#define MAX_LEN 64
#define MAX_TERMS 64

typedef struct {
	char lhs;
	int alt_count;
	char alts[MAX_ALTS][MAX_LEN];
} Production;

static Production prods[MAX_PROD];
static int prod_count = 0;

static char nt_list[MAX_PROD];
static int nt_count = 0;

static char term_list[MAX_TERMS];
static int term_count = 0;

static bool leading_set[MAX_PROD][MAX_TERMS];
static bool trailing_set[MAX_PROD][MAX_TERMS];

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

static int term_index(char t) {
	for (int i = 0; i < term_count; i++) {
		if (term_list[i] == t) {
			return i;
		}
	}
	if (term_count < MAX_TERMS) {
		term_list[term_count] = t;
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

static void add_production(char lhs, const char *alt) {
	int idx = find_prod(lhs);
	if (idx == -1) {
		idx = prod_count++;
		prods[idx].lhs = lhs;
		prods[idx].alt_count = 0;
		nt_index(lhs);
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

static void collect_terminals(void) {
	for (int i = 0; i < prod_count; i++) {
		for (int j = 0; j < prods[i].alt_count; j++) {
			char *alt = prods[i].alts[j];
			for (int k = 0; alt[k] != '\0'; k++) {
				if (!isupper((unsigned char)alt[k])) {
					term_index(alt[k]);
				}
			}
		}
	}
}

static bool add_leading(int A, int t) {
	if (!leading_set[A][t]) {
		leading_set[A][t] = true;
		return true;
	}
	return false;
}

static bool add_trailing(int A, int t) {
	if (!trailing_set[A][t]) {
		trailing_set[A][t] = true;
		return true;
	}
	return false;
}

static bool compute_leading(void) {
	bool changed = false;
	for (int i = 0; i < prod_count; i++) {
		int A = nt_index(prods[i].lhs);
		for (int j = 0; j < prods[i].alt_count; j++) {
			char *alt = prods[i].alts[j];
			if (alt[0] == '\0') {
				continue;
			}
			if (!isupper((unsigned char)alt[0])) {
				int t = term_index(alt[0]);
				if (add_leading(A, t)) {
					changed = true;
				}
			} else {
				int B = nt_index(alt[0]);
				for (int t = 0; t < term_count; t++) {
					if (leading_set[B][t] && add_leading(A, t)) {
						changed = true;
					}
				}
				if (alt[1] != '\0' && !isupper((unsigned char)alt[1])) {
					int t = term_index(alt[1]);
					if (add_leading(A, t)) {
						changed = true;
					}
				}
			}
		}
	}
	return changed;
}

static bool compute_trailing(void) {
	bool changed = false;
	for (int i = 0; i < prod_count; i++) {
		int A = nt_index(prods[i].lhs);
		for (int j = 0; j < prods[i].alt_count; j++) {
			char *alt = prods[i].alts[j];
			int len = (int)strlen(alt);
			if (len == 0) {
				continue;
			}
			if (!isupper((unsigned char)alt[len - 1])) {
				int t = term_index(alt[len - 1]);
				if (add_trailing(A, t)) {
					changed = true;
				}
			} else {
				int B = nt_index(alt[len - 1]);
				for (int t = 0; t < term_count; t++) {
					if (trailing_set[B][t] && add_trailing(A, t)) {
						changed = true;
					}
				}
				if (len > 1 && !isupper((unsigned char)alt[len - 2])) {
					int t = term_index(alt[len - 2]);
					if (add_trailing(A, t)) {
						changed = true;
					}
				}
			}
		}
	}
	return changed;
}

static void print_sets(const char *title, bool set[MAX_PROD][MAX_TERMS]) {
	printf("%s\n", title);
	for (int i = 0; i < nt_count; i++) {
		printf("%c: { ", nt_list[i]);
		bool first = true;
		for (int t = 0; t < term_count; t++) {
			if (set[i][t]) {
				if (!first) {
					printf(", ");
				}
				printf("%c", term_list[t]);
				first = false;
			}
		}
		printf(" }\n");
	}
	printf("\n");
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
		printf("Enter production %d (e.g., E->E+T|T): ", i + 1);
		if (!fgets(line, sizeof(line), stdin)) {
			return 1;
		}
		parse_line(line);
	}

	collect_terminals();

	bool changed = true;
	while (changed) {
		changed = compute_leading();
	}

	changed = true;
	while (changed) {
		changed = compute_trailing();
	}

	print_sets("LEADING sets", leading_set);
	print_sets("TRAILING sets", trailing_set);

	return 0;
}
