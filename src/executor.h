#ifndef EXECUTOR_H
#define EXECUTOR_H

void run_piped(char **cmd1, char **cmd2);
void run_redirect(char **args, char *file, int append);
void run_command(char *cmdstr);

#endif
