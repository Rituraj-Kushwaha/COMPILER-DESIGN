#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_EXPR 256
#define MAX_STACK 256
#define MAX_CODE 128

typedef struct {
	char op;
	char arg1[16];
	char arg2[16];
	char res[16];
} Quad;

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

static int generate_code(const char *postfix, Quad *code) {
	char stack[MAX_STACK][16];
	int top = -1;
	int temp_id = 1;
	int code_count = 0;

	for (int i = 0; postfix[i] != '\0'; i++) {
		char c = postfix[i];
		if (isalnum((unsigned char)c)) {
			char tok[16];
			tok[0] = c;
			tok[1] = '\0';
			strncpy(stack[++top], tok, 15);
			stack[top][15] = '\0';
		} else if (is_operator(c)) {
			if (top < 1 || code_count >= MAX_CODE) {
				return code_count;
			}
			char arg2[16];
			char arg1[16];
			strncpy(arg2, stack[top--], 15);
			arg2[15] = '\0';
			strncpy(arg1, stack[top--], 15);
			arg1[15] = '\0';

			Quad q;
			q.op = c;
			strncpy(q.arg1, arg1, 15);
			strncpy(q.arg2, arg2, 15);
			snprintf(q.res, sizeof(q.res), "t%d", temp_id++);
			code[code_count++] = q;

			strncpy(stack[++top], q.res, 15);
			stack[top][15] = '\0';
		}
	}

	return code_count;
}

static void print_quadruples(const Quad *code, int count) {
	printf("Quadruples\n");
	printf("%-6s %-6s %-6s %-6s %-6s\n", "No", "Op", "Arg1", "Arg2", "Res");
	for (int i = 0; i < count; i++) {
		printf("%-6d %-6c %-6s %-6s %-6s\n", i, code[i].op, code[i].arg1, code[i].arg2, code[i].res);
	}
	printf("\n");
}

static void print_triples(const Quad *code, int count) {
	printf("Triples\n");
	printf("%-6s %-6s %-6s %-6s\n", "No", "Op", "Arg1", "Arg2");
	for (int i = 0; i < count; i++) {
		char arg1[16];
		char arg2[16];
		strncpy(arg1, code[i].arg1, 15);
		strncpy(arg2, code[i].arg2, 15);
		if (code[i].arg1[0] == 't') {
			snprintf(arg1, sizeof(arg1), "(%d)", atoi(code[i].arg1 + 1) - 1);
		}
		if (code[i].arg2[0] == 't') {
			snprintf(arg2, sizeof(arg2), "(%d)", atoi(code[i].arg2 + 1) - 1);
		}
		printf("%-6d %-6c %-6s %-6s\n", i, code[i].op, arg1, arg2);
	}
	printf("\n");
}

static void print_indirect_triples(int count) {
	printf("Indirect Triples\n");
	printf("%-6s %-6s\n", "Ptr", "Triple");
	for (int i = 0; i < count; i++) {
		printf("%-6d (%d)\n", i, i);
	}
	printf("\n");
}

int main(void) {
	char infix[MAX_EXPR];
	char postfix[MAX_EXPR];
	Quad code[MAX_CODE];

	printf("Enter infix expression: ");
	if (!fgets(infix, sizeof(infix), stdin)) {
		return 1;
	}
	infix[strcspn(infix, "\r\n")] = '\0';

	infix_to_postfix(infix, postfix);
	int count = generate_code(postfix, code);

	print_quadruples(code, count);
	print_triples(code, count);
	print_indirect_triples(count);

	printf("Sample input: a+b*c\n");
	return 0;
}
