#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

typedef struct Node {
    char *cmd;
    char *undo;
    struct Node *next;
} Node;

static Node *top = NULL;

static char *dupstr(const char *s) {
    if (!s) return NULL;
    char *p = malloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

void stack_init(void) { top = NULL; }

void stack_push(const char *cmd, const char *undo) {
    Node *n = malloc(sizeof(Node));
    n->cmd = dupstr(cmd);
    n->undo = dupstr(undo);
    n->next = top;
    top = n;
}

char *stack_pop_undo(void) {
    if (!top) return NULL;
    Node *n = top;
    top = n->next;
    char *u = n->undo;
    free(n->cmd);
    free(n);
    return u; // caller must free
}

void stack_print(void) {
    Node *cur = top;
    int idx = 1;
    while (cur) {
        printf("%d %s\n", idx++, cur->cmd);
        cur = cur->next;
    }
}

void stack_free(void) {
    while (top) {
        Node *t = top;
        top = top->next;
        free(t->cmd);
        if (t->undo) free(t->undo);
        free(t);
    }
}
