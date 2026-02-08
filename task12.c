#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR 256
#define MAX_STACK 256
#define MAX_CODE 128

typedef struct {
	char op;
	char arg1[16];
	char arg2[16];
	char res[16];
} Instr;

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

static int generate_three_address(const char *postfix, Instr *code) {
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

			Instr ins;
			ins.op = c;
			strncpy(ins.arg1, arg1, 15);
			strncpy(ins.arg2, arg2, 15);
			snprintf(ins.res, sizeof(ins.res), "t%d", temp_id++);
			code[code_count++] = ins;

			strncpy(stack[++top], ins.res, 15);
			stack[top][15] = '\0';
		}
	}

	return code_count;
}

static void print_three_address(const Instr *code, int count, const char *lhs) {
	for (int i = 0; i < count; i++) {
		printf("t%d = %s %c %s\n", i + 1, code[i].arg1, code[i].op, code[i].arg2);
	}
	if (count > 0) {
		printf("%s = t%d\n", lhs, count);
	}
}

static void print_simple_assembly(const Instr *code, int count, const char *lhs) {
	for (int i = 0; i < count; i++) {
		printf("MOV R0, %s\n", code[i].arg1);
		if (code[i].op == '+') {
			printf("ADD R0, %s\n", code[i].arg2);
		} else if (code[i].op == '-') {
			printf("SUB R0, %s\n", code[i].arg2);
		} else if (code[i].op == '*') {
			printf("MUL R0, %s\n", code[i].arg2);
		} else if (code[i].op == '/') {
			printf("DIV R0, %s\n", code[i].arg2);
		}
		printf("MOV %s, R0\n", code[i].res);
	}
	if (count > 0) {
		printf("MOV %s, t%d\n", lhs, count);
	}
}

int main(void) {
	char input[MAX_EXPR];
	char lhs[16] = {0};
	char rhs[MAX_EXPR] = {0};
	char postfix[MAX_EXPR];
	Instr code[MAX_CODE];

	printf("Enter assignment (e.g., a=b+c*d): ");
	if (!fgets(input, sizeof(input), stdin)) {
		return 1;
	}
	input[strcspn(input, "\r\n")] = '\0';

	char *eq = strchr(input, '=');
	if (!eq) {
		printf("Invalid input. Use format: a=b+c\n");
		return 1;
	}

	*eq = '\0';
	strncpy(lhs, input, sizeof(lhs) - 1);
	strncpy(rhs, eq + 1, sizeof(rhs) - 1);

	infix_to_postfix(rhs, postfix);
	int count = generate_three_address(postfix, code);

	printf("\nThree Address Code\n");
	print_three_address(code, count, lhs);

	printf("\nSimple Assembly\n");
	print_simple_assembly(code, count, lhs);

	printf("\nSample input: x=a+b*c\n");
	return 0;
}
