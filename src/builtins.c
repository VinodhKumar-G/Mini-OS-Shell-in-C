#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "builtins.h"
#include "stack.h"
#include "kv.h"
#include "queue.h"

/* helper: remove operator at pos i and its filename (i and i+1) from argv */
static void remove_token_and_next(char **argv, int i) {
    int j = i;
    while (argv[j+2]) {
        argv[j] = argv[j+2];
        j++;
    }
    argv[j] = NULL;
    argv[j+1] = NULL;
}

/* Prepare redirection for builtins:
   Scans argv for <, >, >> ; opens files and removes those tokens.
   On success, returns 0 and sets *fd_in and *fd_out (or -1).
   On error, prints message and returns -1.
*/
static int prepare_redirection(char **argv, int *fd_in, int *fd_out) {
    *fd_in = -1;
    *fd_out = -1;

    for (int i=0; argv[i]; ++i) {
        if (strcmp(argv[i], "<") == 0) {
            if (!argv[i+1]) { fprintf(stderr, "syntax error: < requires file\n"); return -1; }
            int fd = open(argv[i+1], O_RDONLY);
            if (fd < 0) { perror("open"); return -1; }
            *fd_in = fd;
            remove_token_and_next(argv, i);
            i = -1; /* restart scan since array changed */
        } else if (strcmp(argv[i], ">") == 0) {
            if (!argv[i+1]) { fprintf(stderr, "syntax error: > requires file\n"); return -1; }
            int fd = open(argv[i+1], O_CREAT|O_WRONLY|O_TRUNC, 0644);
            if (fd < 0) { perror("open"); return -1; }
            *fd_out = fd;
            remove_token_and_next(argv, i);
            i = -1;
        } else if (strcmp(argv[i], ">>") == 0) {
            if (!argv[i+1]) { fprintf(stderr, "syntax error: >> requires file\n"); return -1; }
            int fd = open(argv[i+1], O_CREAT|O_WRONLY|O_APPEND, 0644);
            if (fd < 0) { perror("open"); return -1; }
            *fd_out = fd;
            remove_token_and_next(argv, i);
            i = -1;
        }
    }
    return 0;
}

/* helper: join argv back to a single string (not used heavily but available) */
static char *join_argv(char **argv) {
    size_t total = 0;
    for (int i=0; argv[i]; ++i) total += strlen(argv[i]) + 1;
    char *s = malloc(total + 1);
    s[0] = '\0';
    for (int i=0; argv[i]; ++i) {
        strcat(s, argv[i]);
        if (argv[i+1]) strcat(s, " ");
    }
    return s;
}

/* echo with $VAR expansion */
static void builtin_echo(char **argv) {
    for (int i=1; argv[i]; ++i) {
        if (argv[i][0] == '$') {
            char *v = kv_get(argv[i] + 1);
            if (v) { printf("%s", v); free(v); }
        } else {
            printf("%s", argv[i]);
        }
        if (argv[i+1]) printf(" ");
    }
    printf("\n");
}

int run_builtin(char **argv, const char *orig_cmd) {
    (void)orig_cmd; /* currently not used by all builtins */

    if (!argv || !argv[0]) return 0;

    /* Prepare redirection for this builtin (if any).
       Save current fds and dup2 to new ones while the builtin runs.
    */
    int fd_in = -1, fd_out = -1;
    if (prepare_redirection(argv, &fd_in, &fd_out) < 0) {
        return 1; /* redirection error — treat as handled */
    }

    int saved_stdin = -1, saved_stdout = -1;
    if (fd_in != -1) {
        saved_stdin = dup(STDIN_FILENO);
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if (fd_out != -1) {
        saved_stdout = dup(STDOUT_FILENO);
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    /* ---------- Handle builtins ---------- */
    if (strcmp(argv[0], "echo") == 0) {
        builtin_echo(argv);

        /* restore and return */
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "history") == 0) {
        stack_print();
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "cd") == 0) {
        char prev[1024];
        getcwd(prev, sizeof(prev));

        if (argv[1] == NULL) chdir(getenv("HOME"));
        else if (strcmp(argv[1], "-") == 0) {
            char *old = kv_get("OLDPWD");
            if (old) { chdir(old); free(old); }
        } else {
            chdir(argv[1]);
        }

        kv_set("OLDPWD", prev);

        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "pwd") == 0) {
        char buf[1024];
        getcwd(buf, sizeof(buf));
        printf("%s\n", buf);

        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "set") == 0) {
        if (!argv[1]) { fprintf(stderr, "usage: set VAR=value\n"); 
            if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
            if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
            return 1;
        }
        char *eq = strchr(argv[1], '=');
        if (!eq) { fprintf(stderr,"usage: set VAR=value\n");
            if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
            if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
            return 1;
        }
        *eq = '\0';
        kv_set(argv[1], eq+1);

        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "unset") == 0) {
        if (!argv[1]) { fprintf(stderr,"usage: unset VAR\n");
            if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
            if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
            return 1;
        }
        kv_unset(argv[1]);
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "printvars") == 0) {
        kv_print_all();
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "jobs") == 0) {
        queue_print();
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "undo") == 0) {
        char *rec = stack_pop_undo();
        if (!rec) { printf("nothing to undo\n");
            if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
            if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
            return 1;
        }
        if (strncmp(rec, "SET ", 4) == 0) {
            char *rest = rec + 4;
            char *eq = strchr(rest, '=');
            if (eq) {
                *eq = '\0';
                kv_set(rest, eq+1);
            }
        } else if (strncmp(rec, "UNSET ", 6) == 0) {
            char *key = rec + 6;
            kv_unset(key);
        }
        free(rec);
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        return 1;
    }

    if (strcmp(argv[0], "exit") == 0) {
        if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
        if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }
        exit(0);
    }

    /* No builtin matched: restore fds and return 0 so executor runs it */
    if (saved_stdout != -1) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
    if (saved_stdin  != -1) { dup2(saved_stdin,  STDIN_FILENO);  close(saved_stdin); }

    return 0;
}
