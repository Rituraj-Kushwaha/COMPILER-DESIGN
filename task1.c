#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

static const char *C_KEYWORDS[] = {
    "auto","break","case","char","const","continue","default","do","double",
    "else","enum","extern","float","for","goto","if","int","long","register",
    "return","short","signed","sizeof","static","struct","switch","typedef",
    "union","unsigned","void","volatile","while"
};

static bool is_keyword(const char *word) {
    size_t count = sizeof(C_KEYWORDS) / sizeof(C_KEYWORDS[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcmp(word, C_KEYWORDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

int main(void) {
    char filename[256];

    printf("Enter input file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        return 1;
    }
    filename[strcspn(filename, "\r\n")] = '\0';

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    long tokens = 0;
    long keywords = 0;
    long spaces = 0;
    long symbols = 0;

    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c)) {
            spaces++;
            continue;
        }

        if (isalpha(c) || c == '_') {
            char buf[256];
            int i = 0;

            buf[i++] = (char)c;
            while ((c = fgetc(fp)) != EOF && (isalnum(c) || c == '_')) {
                if (i < (int)sizeof(buf) - 1) {
                    buf[i++] = (char)c;
                }
            }
            buf[i] = '\0';
            if (c != EOF) {
                ungetc(c, fp);
            }

            tokens++;
            if (is_keyword(buf)) {
                keywords++;
            }
            continue;
        }

        if (isdigit(c)) {
            while ((c = fgetc(fp)) != EOF && isdigit(c)) {
                /* scan full number */
            }
            if (c != EOF) {
                ungetc(c, fp);
            }
            tokens++;
            continue;
        }

        if (c == '/') {
            int next = fgetc(fp);
            if (next == '/') {
                while ((c = fgetc(fp)) != EOF && c != '\n') {
                    /* skip line comment */
                }
                if (c == '\n') {
                    spaces++;
                }
                continue;
            }
            if (next == '*') {
                int prev = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (prev == '*' && c == '/') {
                        break;
                    }
                    prev = c;
                }
                continue;
            }
            if (next != EOF) {
                ungetc(next, fp);
            }
        }

        symbols++;
        tokens++;
    }

    fclose(fp);

    printf("Total tokens: %ld\n", tokens);
    printf("Keywords: %ld\n", keywords);
    printf("Spaces: %ld\n", spaces);
    printf("Symbols: %ld\n", symbols);

    return 0;
}