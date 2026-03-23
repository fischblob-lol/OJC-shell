#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <sys/ioctl.h>

int last_status = 0;
char elapsed_str[32] = "";

void handle_sigint(int sig) {
  (void)sig;
  last_status = 130;
  write(STDOUT_FILENO, "\n", 1);
  rl_on_new_line();
  rl_replace_line("", 0);
  rl_redisplay();
}

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define RESET   "\033[0m"


void expand_vars(char *input, char *output, int size) {
  int i = 0, j = 0;
  while (input[i] && j < size - 1) {
    if (input[i] == '$') {
      i++;

      if (input[i] == '?') {
        char code[16];
        snprintf(code, sizeof(code), "%d", last_status);
        char *val = code;
        while (*val && j < size - 1) 
          output[j++] = *val++;
        i++;
        continue;
      }
      char var[64];
      int k = 0;

      while (input[i] && input[i] != ' ' && k < 63) {
        var[k++] = input[i++];
      }
      var[k] = '\0';

      char *val = getenv(var);
      if (val) {
        while (*val && j < size - 1) 
          output[j++] = *val++;
      }
    } else {
      output[j++] = input [i++];
    }
  } 
  output[j] = '\0';
}

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

int main(void) {
  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  extern int rl_done;
    char *input;
    char *args[64];

    while (1) {
          char cwd[1024];
          getcwd(cwd, sizeof(cwd));
          char *home = getenv("HOME");
          char display[1024];

          if (strncmp(cwd, home, strlen(home)) == 0) {
            snprintf(display, sizeof(display), "$H%s", cwd + strlen(home));
          } else {
            snprintf(display, sizeof(display), "%s", cwd);
          }

          time_t now = time(NULL);
          char timestr[64];
          struct tm *tm_info = localtime(&now);
          strftime(timestr, sizeof(timestr), "%a %b %d %H:%M:%S", tm_info);

          struct winsize w; 
          ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
          int cols = w.ws_col;

          int spaces = cols - strlen(display) - strlen(timestr);
          if (spaces < 1) spaces = 1;

          int spaces1 = (cols - strlen(display) - strlen(elapsed_str) - strlen(timestr)) / 2;
          if (spaces1 < 1) spaces1 = 1;

          int spaces2 = cols - strlen(display) - spaces1 - strlen(elapsed_str) - strlen(timestr);
          if (spaces2 < 1) spaces2 = 1;

          printf(GREEN "%s" RESET, display); 
          for (int s = 0; s < spaces1; s++) 
            printf(" ");
          printf(GREEN "%s" RESET, elapsed_str);
          for (int s = 0; s < spaces2; s++) 
            printf(" ");
          printf(GREEN "%s\n" RESET, timestr);

      char prompt[64];
        if (last_status == 0)
            snprintf(prompt, sizeof(prompt), GREEN "[%d]OJC> " RESET, last_status);
        else
            snprintf(prompt, sizeof(prompt), RED "[%d]OJC> " RESET, last_status);
           
        input = readline(prompt);

        if (!input) {
          printf("just use 'exit' to leave OJC-sh!\n");
          continue;
        } 

        if (*input) add_history(input);

        char expanded[1024];
        expand_vars(input, expanded, sizeof(expanded));
        input[strcspn(input, "\n")] = '\0';

        int i = 0;
        args[i] = strtok(expanded, " ");
        while (args[i] != NULL) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        if (args[0] == NULL) {
          free(input);
          continue;
        }
      
        char **pipe_pos = NULL;
        for (int p = 0; args[p] != NULL; p++) {
        if (strcmp(args[p], "|") == 0) {
        args[p] = NULL;
        pipe_pos = &args[p+1];
        break;
        }
      }
    
        if (pipe_pos != NULL) {
          run_piped(args, pipe_pos);
          free(input);
          continue;
        }

        char *redirect_file = NULL;
        int append = 0;

        for (int r = 0; args[r] != NULL; r++) {
          if (strcmp(args[r], ">>") == 0) {
            args[r] = NULL;
            redirect_file = args[r+1];
            append = 1;
            break;
          }
          if (strcmp(args[r], ">") == 0) {
            args[r] = NULL;
            redirect_file = args[r+1];
            append = 0;
            break;
          }
        }

        if (redirect_file != NULL) {
          run_redirect(args, redirect_file, append);
          free(input);
          continue;
        }

        if (strcmp(args[0], "export") == 0) {
          if (args[1] == NULL) {
            printf("exp.use: export VAR=value\n");
            last_status = 1;
          } else {
            putenv(args[1]);
            last_status = 0;
          }
          free(input);
          continue;
        }

        if (args[0] == NULL) { 
          free(input);
          continue;
        }

        if (strcmp(args[0], "who?") == 0) {
          printf("        >_ \n OJC // SHELL\n \n [ built from silence ]\n [ executed in control ]\n \n      — ojclicks —\n");
          last_status = 0;
          free(input);
          continue;
        }

        if (strcmp(args[0], "ojcshfetch") == 0) {
          printf("\n"
         "   ██████╗      ██╗ ██████╗      ███████╗██╗  ██╗███████╗██╗     ██╗\n"
         "  ██╔═══██╗     ██║██╔════╝      ██╔════╝██║  ██║██╔════╝██║     ██║\n"
         "  ██║   ██║     ██║██║           ███████╗███████║█████╗  ██║     ██║\n"
         "  ██║   ██║██   ██║██║           ╚════██║██╔══██║██╔══╝  ██║     ██║\n"
         "  ╚██████╔╝╚█████╔╝╚██████╗      ███████║██║  ██║███████╗███████╗███████╗\n"
         "   ╚═════╝  ╚════╝  ╚═════╝      ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝\n"
         "\n"
         "                >_  O J C   S H E L L  _<\n\n"
                                              );
         last_status = 0;
          free(input);
          continue;
        }

        if (strcmp(args[0], "help") == 0) {
          printf("try this commans...\n'who?'\n'hello'\n'about'\n'ojcshfetch'\n");
          last_status = 0;
          free(input);
          continue;
        }

        if (strcmp(args[0], "hello") == 0) {
            printf("hello from OJC-shell a vision for OJclicks os...\n");
            last_status = 0;
            free(input);
            continue;
        }
        if (strcmp(args[0], "about") == 0) {
            printf("OJC-shell — a minimal Unix-like shell written in C\n");
            printf("Part of the OJclicks OS journey\n");
            printf("Not here to compete with Bash...\n");
            printf("just here to understand it\n");
            printf("works...\nbreaks...\nteaches...\n");
            printf("If it crashes, it's a lesson.\n");
            printf("If it works, question it.\n");
            last_status = 0;
            free(input);
            continue;
        }
        if (strcmp(args[0], "exit") == 0) {
            printf("as you like...\n");
            free(input);
            break;
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                chdir(getenv("HOME"));
                last_status = 0;
            } else {
                if (chdir(args[1]) != 0) {
                    printf("W: did u type it right? %s\n", args[1]);
                    last_status = 1;
                } else {
                    last_status = 0;
                }
            }
            free(input);
            continue;
        }

        struct timespec start, end; 
        clock_gettime(CLOCK_MONOTONIC, &start);

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            printf("not found... %s\n", args[0]);
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
        free(input);
    }
    return 0;
}
