#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) 
{
  char input[1024];
  char *args[64];

  while (1) {
    printf("OJC> ");
    fflush(stdout);

    if (!fgets(input, sizeof(input), stdin)) break; 
        input[strcspn(input, "\n")] = '\0';

        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL) {
          i++;
          args[i] = strtok(NULL, " ");
        }

        if (args == NULL) continue;

        if (strcmp(args[0], "cd") == 0) {
          if (args[1] == NULL) {
            chdir(getenv("HOME"));
          } else {
            if (chdir(args[1]) != 0) {
              printf("W: did u type it right? %s\n", args[1]);
            }
          }
          continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
          execvp(args[0], args);
          printf("not found... %s\n", args[0]);
          exit(1);
          
        } else {
          wait(NULL);
        }

  }

  return 0;
}
