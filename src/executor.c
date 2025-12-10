#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include "executor.h"
#include "queue.h"

// check and remove trailing "&"
static int is_background(char **argv) {
    int i = 0;
    while (argv[i]) i++;
    if (i == 0) return 0;

    if (strcmp(argv[i - 1], "&") == 0) {
        argv[i - 1] = NULL;
        return 1;
    }
    return 0;
}

// find pipe position
static int find_pipe(char **argv) {
    for (int i = 0; argv[i]; i++)
        if (strcmp(argv[i], "|") == 0) return i;
    return -1;
}

void execute_command(char **argv, const char *orig_cmd) {
    if (!argv || !argv[0]) return;

    int bg = is_background(argv);

    // ------------------ PIPE HANDLING ------------------
    int pipe_pos = find_pipe(argv);
    if (pipe_pos != -1) {
        char *left[64], *right[64];
        int L = 0, R = 0;

        for (int i=0; i<pipe_pos; i++) left[L++] = argv[i];
        left[L] = NULL;

        for (int i = pipe_pos+1; argv[i]; i++) right[R++] = argv[i];
        right[R] = NULL;

        int pfd[2];
        pipe(pfd);

        if (fork() == 0) {
            dup2(pfd[1], STDOUT_FILENO);
            close(pfd[0]);
            close(pfd[1]);
            execvp(left[0], left);
            perror("execvp");
            exit(1);
        }

        if (fork() == 0) {
            dup2(pfd[0], STDIN_FILENO);
            close(pfd[0]);
            close(pfd[1]);
            execvp(right[0], right);
            perror("execvp");
            exit(1);
        }

        close(pfd[0]);
        close(pfd[1]);
        wait(NULL);
        wait(NULL);
        return;
    }

    // ------------------ REDIRECTION FIX ------------------
    int fd_in = -1, fd_out = -1;

    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "<") == 0) {
            if (!argv[i+1]) { fprintf(stderr, "syntax error: < missing file\n"); return; }
            fd_in = open(argv[i+1], O_RDONLY);
            if (fd_in < 0) { perror("open"); return; }

            // Remove operator AND filename
            for (int j=i; argv[j]; j++) argv[j] = argv[j+2];
            i--; // reprocess new token at this position
        }
        else if (strcmp(argv[i], ">") == 0) {
            if (!argv[i+1]) { fprintf(stderr, "syntax error: > missing file\n"); return; }
            fd_out = open(argv[i+1], O_CREAT|O_WRONLY|O_TRUNC, 0644);
            if (fd_out < 0) { perror("open"); return; }

            // Remove operator AND filename
            for (int j=i; argv[j]; j++) argv[j] = argv[j+2];
            i--;
        }
        else if (strcmp(argv[i], ">>") == 0) {
            if (!argv[i+1]) { fprintf(stderr, "syntax error: >> missing file\n"); return; }
            fd_out = open(argv[i+1], O_CREAT|O_WRONLY|O_APPEND, 0644);
            if (fd_out < 0) { perror("open"); return; }

            // Remove operator AND filename
            for (int j=i; argv[j]; j++) argv[j] = argv[j+2];
            i--;
        }
    }

    // ------------------ FORK AND EXEC ------------------
    pid_t pid = fork();

    if (pid == 0) {
        if (fd_in  != -1) { dup2(fd_in,  STDIN_FILENO); close(fd_in); }
        if (fd_out != -1) { dup2(fd_out, STDOUT_FILENO); close(fd_out); }

        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    }

    if (!bg) {
        waitpid(pid, NULL, 0);
    } else {
        printf("[%d] %s &\n", pid, orig_cmd);
        queue_add(pid, orig_cmd);
    }
}
