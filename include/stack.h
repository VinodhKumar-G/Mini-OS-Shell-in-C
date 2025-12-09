#ifndef STACK_H
#define STACK_H

typedef struct HistoryNode {
    char *cmd;
    char *undo;
    struct HistoryNode *next;
} HistoryNode;

void stack_init();
void stack_push(const char *cmd, const char *undo);
char *stack_pop_undo();
void stack_print();
void stack_free();

#endif
