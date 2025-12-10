#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

static char *my_strdup(const char *s) {
    char *p = malloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

static int is_op(char c) {
    return (c == '<' || c == '>' || c == '|' || c == '&');
}

char **tokenize(const char *line) {
    int cap = 32;
    char **tokens = malloc(sizeof(char*) * cap);
    int count = 0;

    int i = 0, n = strlen(line);

    while (i < n) {
        while (i < n && isspace(line[i])) i++;
        if (i >= n) break;

        char buf[256];
        int b = 0;

        // quote
        if (line[i] == '"') {
            i++;
            while (i < n && line[i] != '"') buf[b++] = line[i++];
            if (i < n) i++;
        }
        else if (line[i] == '\'') {
            i++;
            while (i < n && line[i] != '\'') buf[b++] = line[i++];
            if (i < n) i++;
        }
        // operators possibly stuck to text (like >file)
        else if (is_op(line[i])) {
            buf[b++] = line[i++];
            // >> operator
            if (buf[0] == '>' && line[i] == '>') buf[b++] = line[i++];
        }
        else {
            while (i < n && !isspace(line[i]) && !is_op(line[i])) {
                buf[b++] = line[i++];
            }
        }

        buf[b] = '\0';

        // If operator was glued to text (e.g., "hello>file")
        // we must split it: hello > file
        if (!is_op(buf[0])) {
            int pos = 0;
            while (buf[pos] && !is_op(buf[pos])) pos++;

            if (buf[pos] && is_op(buf[pos])) {
                // split
                char before[256], op[3], after[256];

                strncpy(before, buf, pos);
                before[pos] = '\0';

                op[0] = buf[pos];
                op[1] = '\0';
                if (buf[pos]=='>' && buf[pos+1]=='>') {
                    op[1] = '>';
                    op[2] = '\0';
                    pos++;
                }

                strcpy(after, buf + pos + 1);

                if (strlen(before) > 0) tokens[count++] = my_strdup(before);
                tokens[count++] = my_strdup(op);
                if (strlen(after) > 0) tokens[count++] = my_strdup(after);

                continue;
            }
        }

        tokens[count++] = my_strdup(buf);

        if (count >= cap - 1) {
            cap *= 2;
            tokens = realloc(tokens, sizeof(char*) * cap);
        }
    }

    tokens[count] = NULL;
    return tokens;
}

void free_tokens(char **tokens) {
    if (!tokens) return;
    for (int i = 0; tokens[i]; i++) free(tokens[i]);
    free(tokens);
}
