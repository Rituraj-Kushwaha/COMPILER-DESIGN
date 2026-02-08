#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR 256
#define MAX_STACK 256

static int precedence(char op) {
	if (op == '+' || op == '-') return 1;
	if (op == '*' || op == '/') return 2;
	if (op == '^') return 3;
	return 0;
}

static int is_operator(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

static void reverse_str(char *s) {
	int i = 0;
	int j = (int)strlen(s) - 1;
	while (i < j) {
		char tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		i++;
		j--;
	}
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
				   (precedence(stack[top]) > precedence(c) ||
					(precedence(stack[top]) == precedence(c) && c != '^'))) {
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

static void infix_to_prefix(const char *infix, char *prefix) {
	char temp[MAX_EXPR];
	strncpy(temp, infix, sizeof(temp) - 1);
	temp[sizeof(temp) - 1] = '\0';
	reverse_str(temp);

	for (int i = 0; temp[i] != '\0'; i++) {
		if (temp[i] == '(') {
			temp[i] = ')';
		} else if (temp[i] == ')') {
			temp[i] = '(';
		}
	}

	char postfix[MAX_EXPR];
	infix_to_postfix(temp, postfix);
	reverse_str(postfix);
	strncpy(prefix, postfix, MAX_EXPR - 1);
	prefix[MAX_EXPR - 1] = '\0';
}

int main(void) {
	char infix[MAX_EXPR];
	char postfix[MAX_EXPR];
	char prefix[MAX_EXPR];

	printf("Enter infix expression: ");
	if (!fgets(infix, sizeof(infix), stdin)) {
		return 1;
	}
	infix[strcspn(infix, "\r\n")] = '\0';

	infix_to_postfix(infix, postfix);
	infix_to_prefix(infix, prefix);

	printf("Postfix: %s\n", postfix);
	printf("Prefix: %s\n", prefix);

	printf("\nSample input: (a+b)*c-d/e\n");
	return 0;
}
