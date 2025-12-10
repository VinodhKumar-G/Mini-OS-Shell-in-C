#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "parser.h"
#include "builtins.h"
#include "executor.h"
#include "stack.h"
#include "queue.h"
#include "kv.h"

void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        queue_remove(pid);
    }
}

int main(void) {
    char *line = NULL;
    size_t len = 0;

    stack_init();
    kv_init();
    queue_init();

    signal(SIGCHLD, sigchld_handler);

    while (1) {
        printf("psh> ");
        fflush(stdout);

        ssize_t n = getline(&line, &len, stdin);
        if (n == -1) { printf("\n"); break; }
        if (n>0 && line[n-1]=='\n') line[n-1] = '\0';
        if (strlen(line)==0) continue;

        stack_push(line, NULL);

        char **argv = tokenize(line);
        if (!argv || !argv[0]) { free_tokens(argv); continue; }

        if (run_builtin(argv, line)) { free_tokens(argv); continue; }

        execute_command(argv, line);

        free_tokens(argv);
    }

    free(line);
    stack_free();
    kv_free();
    queue_free();
    return 0;
}
