#ifndef STACK_H
#define STACK_H

void stack_init(void);
void stack_push(const char *cmd, const char *undo);
char *stack_pop_undo(void);
void stack_print(void);
void stack_free(void);

#endif
