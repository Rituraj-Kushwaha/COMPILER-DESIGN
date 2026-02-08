#include <stdio.h>
#include <string.h>

#define MAX_INPUT 128
#define MAX_STACK 128

static const char *PRODS[] = {
	"E->E+E",
	"E->E*E",
	"E->(E)",
	"E->id"
};

static int prod_count = 4;

static void print_state(const char *stack, const char *input, const char *action) {
	printf("%-20s %-20s %-20s\n", stack, input, action);
}

static int match_handle(char *stack, int *top) {
	if (*top < 0) {
		return 0;
	}

	if (*top >= 1 && stack[*top] == 'd' && stack[*top - 1] == 'i') {
		stack[*top - 1] = 'E';
		(*top)--;
		stack[*top + 1] = '\0';
		return 4;
	}

	if (*top >= 2 && stack[*top] == 'E' && stack[*top - 1] == '+' && stack[*top - 2] == 'E') {
		stack[*top - 2] = 'E';
		(*top) -= 2;
		stack[*top + 1] = '\0';
		return 1;
	}

	if (*top >= 2 && stack[*top] == 'E' && stack[*top - 1] == '*' && stack[*top - 2] == 'E') {
		stack[*top - 2] = 'E';
		(*top) -= 2;
		stack[*top + 1] = '\0';
		return 2;
	}

	if (*top >= 2 && stack[*top] == ')' && stack[*top - 1] == 'E' && stack[*top - 2] == '(') {
		stack[*top - 2] = 'E';
		(*top) -= 2;
		stack[*top + 1] = '\0';
		return 3;
	}

	return 0;
}

int main(void) {
	char input[MAX_INPUT];
	char stack[MAX_STACK];
	int top = -1;

	printf("Enter input string (use id, +, *, (, )): ");
	if (!fgets(input, sizeof(input), stdin)) {
		return 1;
	}
	input[strcspn(input, "\r\n")] = '\0';
	strcat(input, "$" );

	printf("\n%-20s %-20s %-20s\n", "Stack", "Input", "Action");
	printf("------------------------------------------------------------\n");

	stack[0] = '\0';
	int ip = 0;

	while (1) {
		int reduced = 0;
		int rule = match_handle(stack, &top);
		if (rule != 0) {
			char action[32];
			snprintf(action, sizeof(action), "Reduce %s", PRODS[rule - 1]);
			print_state(stack, &input[ip], action);
			reduced = 1;
		}

		if (reduced) {
			continue;
		}

		if (input[ip] == '$' && top == 0 && stack[top] == 'E') {
			print_state(stack, &input[ip], "Accept");
			break;
		}

		if (input[ip] == '$') {
			print_state(stack, &input[ip], "Reject");
			break;
		}

		if (input[ip] == 'i' && input[ip + 1] == 'd') {
			stack[++top] = 'i';
			stack[++top] = 'd';
			stack[top + 1] = '\0';
			print_state(stack, &input[ip], "Shift id");
			ip += 2;
			continue;
		}

		stack[++top] = input[ip];
		stack[top + 1] = '\0';
		char action[32];
		snprintf(action, sizeof(action), "Shift %c", input[ip]);
		print_state(stack, &input[ip], action);
		ip++;
	}

	printf("\nSample input: id+id*id\n");
	return 0;
}
