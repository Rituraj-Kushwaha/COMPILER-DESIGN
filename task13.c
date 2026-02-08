#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR 256
#define MAX_STACK 256
#define MAX_NODES 128
#define MAX_LABELS 8

typedef struct {
	char op;
	int left;
	int right;
	char labels[MAX_LABELS][16];
	int label_count;
	char value[16];
} Node;

static Node nodes[MAX_NODES];
static int node_count = 0;

static int precedence(char op) {
	if (op == '+' || op == '-') return 1;
	if (op == '*' || op == '/') return 2;
	return 0;
}

static int is_operator(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/';
}

static void infix_to_postfix(const char *infix, char *postfix) {
	char stack[MAX_STACK];
	int top = -1;
	int j = 0;

	for (int i = 0; infix[i] != '\0'; i++) {
		char c = infix[i];
		if (isspace((unsigned char)c)) {
			continue;
		}
		if (isalnum((unsigned char)c)) {
			postfix[j++] = c;
		} else if (c == '(') {
			stack[++top] = c;
		} else if (c == ')') {
			while (top >= 0 && stack[top] != '(') {
				postfix[j++] = stack[top--];
			}
			if (top >= 0 && stack[top] == '(') {
				top--;
			}
		} else if (is_operator(c)) {
			while (top >= 0 && is_operator(stack[top]) &&
				   precedence(stack[top]) >= precedence(c)) {
				postfix[j++] = stack[top--];
			}
			stack[++top] = c;
		}
	}

	while (top >= 0) {
		postfix[j++] = stack[top--];
	}
	postfix[j] = '\0';
}

static int find_leaf(const char *value) {
	for (int i = 0; i < node_count; i++) {
		if (nodes[i].op == '\0' && strcmp(nodes[i].value, value) == 0) {
			return i;
		}
	}
	return -1;
}

static int find_op_node(char op, int left, int right) {
	for (int i = 0; i < node_count; i++) {
		if (nodes[i].op == op && nodes[i].left == left && nodes[i].right == right) {
			return i;
		}
	}
	return -1;
}

static int add_leaf(const char *value) {
	int idx = find_leaf(value);
	if (idx != -1) {
		return idx;
	}
	if (node_count >= MAX_NODES) {
		return -1;
	}
	nodes[node_count].op = '\0';
	nodes[node_count].left = -1;
	nodes[node_count].right = -1;
	nodes[node_count].label_count = 0;
	strncpy(nodes[node_count].value, value, 15);
	nodes[node_count].value[15] = '\0';
	return node_count++;
}

static int add_op_node(char op, int left, int right) {
	int idx = find_op_node(op, left, right);
	if (idx != -1) {
		return idx;
	}
	if (node_count >= MAX_NODES) {
		return -1;
	}
	nodes[node_count].op = op;
	nodes[node_count].left = left;
	nodes[node_count].right = right;
	nodes[node_count].label_count = 0;
	nodes[node_count].value[0] = '\0';
	return node_count++;
}

static void add_label(int node, const char *label) {
	if (node < 0 || node >= node_count) {
		return;
	}
	for (int i = 0; i < nodes[node].label_count; i++) {
		if (strcmp(nodes[node].labels[i], label) == 0) {
			return;
		}
	}
	if (nodes[node].label_count < MAX_LABELS) {
		strncpy(nodes[node].labels[nodes[node].label_count], label, 15);
		nodes[node].labels[nodes[node].label_count][15] = '\0';
		nodes[node].label_count++;
	}
}

static int build_dag_from_postfix(const char *postfix) {
	int stack[MAX_STACK];
	int top = -1;

	for (int i = 0; postfix[i] != '\0'; i++) {
		char c = postfix[i];
		if (isalnum((unsigned char)c)) {
			char tok[2] = { c, '\0' };
			int node = add_leaf(tok);
			stack[++top] = node;
		} else if (is_operator(c)) {
			if (top < 1) {
				return -1;
			}
			int right = stack[top--];
			int left = stack[top--];
			int node = add_op_node(c, left, right);
			stack[++top] = node;
		}
	}

	if (top != 0) {
		return -1;
	}
	return stack[top];
}

static void print_dag(void) {
	printf("DAG Nodes\n");
	printf("%-4s %-4s %-6s %-6s %-14s\n", "Id", "Op", "Left", "Right", "Labels/Value");
	for (int i = 0; i < node_count; i++) {
		if (nodes[i].op == '\0') {
			printf("%-4d %-4s %-6s %-6s %-14s\n", i, "-", "-", "-", nodes[i].value);
		} else {
			printf("%-4d %-4c %-6d %-6d ", i, nodes[i].op, nodes[i].left, nodes[i].right);
			if (nodes[i].label_count == 0) {
				printf("-\n");
			} else {
				for (int j = 0; j < nodes[i].label_count; j++) {
					printf("%s", nodes[i].labels[j]);
					if (j + 1 < nodes[i].label_count) {
						printf(",");
					}
				}
				printf("\n");
			}
		}
	}
	printf("\n");
}

int main(void) {
	int n = 0;
	char line[256];

	printf("Enter number of assignments: ");
	if (scanf("%d", &n) != 1 || n <= 0) {
		return 1;
	}
	fgets(line, sizeof(line), stdin);

	for (int i = 0; i < n; i++) {
		char lhs[16];
		char rhs[MAX_EXPR];
		char postfix[MAX_EXPR];

		printf("Enter assignment %d (e.g., a=b+c): ", i + 1);
		if (!fgets(line, sizeof(line), stdin)) {
			return 1;
		}
		line[strcspn(line, "\r\n")] = '\0';

		char *eq = strchr(line, '=');
		if (!eq) {
			return 1;
		}
		*eq = '\0';
		strncpy(lhs, line, sizeof(lhs) - 1);
		lhs[sizeof(lhs) - 1] = '\0';
		strncpy(rhs, eq + 1, sizeof(rhs) - 1);
		rhs[sizeof(rhs) - 1] = '\0';

		infix_to_postfix(rhs, postfix);
		int root = build_dag_from_postfix(postfix);
		if (root < 0) {
			return 1;
		}
		add_label(root, lhs);
	}

	print_dag();

	printf("Sample input:\n");
	printf("3\n");
	printf("t1=a+b\n");
	printf("t2=a+b\n");
	printf("t3=t1*c\n");

	return 0;
}
