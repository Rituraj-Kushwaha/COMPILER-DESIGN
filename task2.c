#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_REGEX 512
#define MAX_POSTFIX 1024
#define MAX_STATES 1024
#define MAX_TRANS 4096

typedef struct {
    int from;
    int to;
    char symbol; /* 0 means epsilon */
} Transition;

typedef struct {
    int start;
    int accept;
} Fragment;

static Transition transitions[MAX_TRANS];
static int trans_count = 0;

static int next_state = 0;

static bool is_operator(char c) {
    return c == '|' || c == '.' || c == '*' || c == '+' || c == '?';
}

static bool is_operand(char c) {
    return (isalnum((unsigned char)c) || c == '_');
}

static int precedence(char c) {
    if (c == '|' ) return 1;
    if (c == '.' ) return 2;
    return 0;
}

static void add_transition(int from, int to, char symbol) {
    if (trans_count < MAX_TRANS) {
        transitions[trans_count].from = from;
        transitions[trans_count].to = to;
        transitions[trans_count].symbol = symbol;
        trans_count++;
    }
}

static char *insert_concat(const char *regex, char *out) {
    int j = 0;
    for (int i = 0; regex[i] != '\0'; i++) {
        char c1 = regex[i];
        if (isspace((unsigned char)c1)) {
            continue;
        }
        out[j++] = c1;

        char c2 = regex[i + 1];
        if (c2 == '\0') {
            continue;
        }
        if (isspace((unsigned char)c2)) {
            continue;
        }

        if ((is_operand(c1) || c1 == ')' || c1 == '*' || c1 == '+' || c1 == '?') &&
            (is_operand(c2) || c2 == '(')) {
            out[j++] = '.';
        }
    }
    out[j] = '\0';
    return out;
}

static char *to_postfix(const char *regex, char *postfix) {
    char stack[MAX_POSTFIX];
    int top = -1;
    int j = 0;

    for (int i = 0; regex[i] != '\0'; i++) {
        char c = regex[i];
        if (is_operand(c)) {
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
        } else if (c == '*' || c == '+' || c == '?') {
            postfix[j++] = c;
        } else if (c == '.' || c == '|') {
            while (top >= 0 && stack[top] != '(' &&
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
    return postfix;
}

static Fragment build_nfa(const char *postfix) {
    Fragment stack[MAX_POSTFIX];
    int top = -1;

    for (int i = 0; postfix[i] != '\0'; i++) {
        char c = postfix[i];

        if (is_operand(c)) {
            int s = next_state++;
            int e = next_state++;
            add_transition(s, e, c);
            stack[++top] = (Fragment){s, e};
        } else if (c == '.') {
            Fragment b = stack[top--];
            Fragment a = stack[top--];
            add_transition(a.accept, b.start, 0);
            stack[++top] = (Fragment){a.start, b.accept};
        } else if (c == '|') {
            Fragment b = stack[top--];
            Fragment a = stack[top--];
            int s = next_state++;
            int e = next_state++;
            add_transition(s, a.start, 0);
            add_transition(s, b.start, 0);
            add_transition(a.accept, e, 0);
            add_transition(b.accept, e, 0);
            stack[++top] = (Fragment){s, e};
        } else if (c == '*') {
            Fragment a = stack[top--];
            int s = next_state++;
            int e = next_state++;
            add_transition(s, a.start, 0);
            add_transition(s, e, 0);
            add_transition(a.accept, a.start, 0);
            add_transition(a.accept, e, 0);
            stack[++top] = (Fragment){s, e};
        } else if (c == '+') {
            Fragment a = stack[top--];
            int s = next_state++;
            int e = next_state++;
            add_transition(s, a.start, 0);
            add_transition(a.accept, a.start, 0);
            add_transition(a.accept, e, 0);
            stack[++top] = (Fragment){s, e};
        } else if (c == '?') {
            Fragment a = stack[top--];
            int s = next_state++;
            int e = next_state++;
            add_transition(s, a.start, 0);
            add_transition(s, e, 0);
            add_transition(a.accept, e, 0);
            stack[++top] = (Fragment){s, e};
        }
    }

    return stack[top];
}

int main(void) {
    char input[MAX_REGEX];
    char with_concat[MAX_POSTFIX];
    char postfix[MAX_POSTFIX];

    printf("Enter regular expression: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 1;
    }
    input[strcspn(input, "\r\n")] = '\0';

    insert_concat(input, with_concat);
    to_postfix(with_concat, postfix);

    Fragment nfa = build_nfa(postfix);

    printf("\nStart state: %d\n", nfa.start);
    printf("Accept state: %d\n\n", nfa.accept);

    printf("State Transition Table\n");
    printf("%-8s %-8s %-8s\n", "From", "Symbol", "To");
    printf("------------------------\n");
    for (int i = 0; i < trans_count; i++) {
        if (transitions[i].symbol == 0) {
            printf("%-8d %-8s %-8d\n", transitions[i].from, "eps", transitions[i].to);
        } else {
            char sym[2] = { transitions[i].symbol, '\0' };
            printf("%-8d %-8s %-8d\n", transitions[i].from, sym, transitions[i].to);
        }
    }

    return 0;
}