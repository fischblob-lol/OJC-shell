#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

int last_status = 0;

void handle_sigint(int sig) {
  (void)sig;
  last_status = 130;
  printf("\n");
  rl_on_new_line();
  rl_replace_line("", 0);
  rl_cleanup_after_signal();
  rl_free_line_state();
  // rl_redisplay();
  rl_done = 1;
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

int main(void) {
  signal(SIGINT, handle_sigint);
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

      printf( GREEN "%s\n" RESET, display);
      char prompt[64];
        if (last_status == 0)
            snprintf(prompt, sizeof(prompt), GREEN "[%d]OJC> " RESET, last_status);
        else
            snprintf(prompt, sizeof(prompt), RED "[%d]OJC> " RESET, last_status);
           
        input = readline(prompt);

        if (!input) {
          printf("just use 'exit' to OJC shell!\n");
          continue;
        } 

        if (!input || (*input == '\0' && last_status == 130)) {
          if (input) free(input);
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

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            printf("not found... %s\n", args[0]);
            // perror(args[0]);
            exit(127);
        } else {
            int status;
            // wait(&status);
            waitpid(pid, &status, 0);
            last_status = WEXITSTATUS(status);
        }
        free(input);
    }
    return 0;
}
