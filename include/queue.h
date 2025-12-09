#ifndef QUEUE_H
#define QUEUE_H
#include <sys/types.h>

void queue_init(void);
void queue_add(pid_t pid, const char *cmd);
void queue_remove(pid_t pid);
void queue_print(void);
void queue_free(void);

#endif
