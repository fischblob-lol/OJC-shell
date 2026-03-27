#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include "shell.h"
#include "parser.h"
#include "executor.h"
#include "builtins.h"

void run_piped(char **cmd1, char **cmd2) {
  int pipefd[2];
  pipe(pipefd);
  pid_t pid1 = fork();
  if (pid1 == 0) {
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(cmd1[0], cmd1);
    exit(127);
  }
  pid_t pid2 = fork();
  if (pid2 == 0) {
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(cmd2[0], cmd2);
    exit(127);
  }

  close(pipefd[0]);
  close(pipefd[1]);
  waitpid(pid1, NULL, 0);
  int status;
  waitpid(pid2, &status, 0);
  if (WIFEXITED(status))
    last_status = WEXITSTATUS(status);
  else
    last_status = 128 + WTERMSIG(status);
}

void run_redirect(char **args, char *file, int append) {
  pid_t pid = fork();
  if (pid == 0) {
    int fd;
    if (append) 
    fd = open(file, O_WRONLY|O_CREAT|O_APPEND, 0664);
    else 
    fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0664);

    dup2(fd, STDOUT_FILENO);
    close(fd);
    execvp(args[0], args);
    exit(127);
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
      last_status = WEXITSTATUS(status);
    else
      last_status = 128 + WTERMSIG(status);
  }
}

// void run_command()
void run_command(char *cmdstr) {
    char expanded[1024];
    expand_vars(cmdstr, expanded, sizeof(expanded));

    char *args[64];
    parse_input(expanded, args);

    for (int k = 0; args[k] != NULL; k++) {
        char temp[1024];
        expand_tilde(args[k], temp, sizeof(temp));
        strcpy(args[k], temp);
    }

    if (args[0] == NULL) return;

    /* builtins */
    if (run_builtin(args)) return;

    /* pipes */
    char **pipe_pos = NULL;
    for (int p = 0; args[p] != NULL; p++) {
        if (strcmp(args[p], "|") == 0) {
            args[p] = NULL;
            pipe_pos = &args[p + 1];
            break;
        }
    }
    if (pipe_pos != NULL) {
        run_piped(args, pipe_pos);
        return;
    }

    /* redirects */
    char *redirect_file = NULL;
    int append = 0;
    for (int r = 0; args[r] != NULL; r++) {
        if (strcmp(args[r], ">>") == 0) {
            args[r] = NULL;
            redirect_file = args[r + 1];
            append = 1;
            break;
        }
        if (strcmp(args[r], ">") == 0) {
            args[r] = NULL;
            redirect_file = args[r + 1];
            append = 0;
            break;
        }
    }
    if (redirect_file != NULL) {
        run_redirect(args, redirect_file, append);
        return;
    }

    /* fork/exec */
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        fprintf(stderr, "not found... %s\n", args[0]);
        exit(127);
    } else {
        int status;
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) +
                         (end.tv_nsec - start.tv_nsec) / 1e9;
        snprintf(elapsed_str, sizeof(elapsed_str), "%.3fs", elapsed);
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else
            last_status = 128 + WTERMSIG(status);
    }
}
