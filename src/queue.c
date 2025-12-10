#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "queue.h"

typedef struct Job {
    pid_t pid;
    char *cmd;
    struct Job *next;
} Job;

static Job *jobs = NULL;

static char *dupstr(const char *s) {
    if (!s) return NULL;
    char *p = malloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

void queue_init(void) { jobs = NULL; }

void queue_add(pid_t pid, const char *cmd) {
    Job *n = malloc(sizeof(Job));
    n->pid = pid;
    n->cmd = dupstr(cmd);
    n->next = jobs;
    jobs = n;
}

void queue_remove(pid_t pid) {
    Job *cur = jobs, *prev = NULL;
    while (cur) {
        if (cur->pid == pid) {
            if (prev) prev->next = cur->next;
            else jobs = cur->next;
            free(cur->cmd);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void queue_print(void) {
    Job *cur = jobs;
    while (cur) {
        printf("[%d] %s\n", (int)cur->pid, cur->cmd);
        cur = cur->next;
    }
}

void queue_free(void) {
    while (jobs) {
        Job *t = jobs;
        jobs = jobs->next;
        free(t->cmd);
        free(t);
    }
}
