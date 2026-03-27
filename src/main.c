#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include "shell.h"
#include "parser.h"
#include "executor.h"

#define RED   "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

char histfile[1024];
int  last_status = 0;
char elapsed_str[32] = "";

void handle_sigint(int sig) {
    (void)sig;
    last_status = 130;
    write(STDOUT_FILENO, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void save_history(void) { write_history(histfile); }

int main(void) {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    snprintf(histfile, sizeof(histfile), "%s/.ojchis", getenv("HOME"));
    read_history(histfile);
    atexit(save_history);

    char *input;

    while (1) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        char *home = getenv("HOME");
        char display[1024];

        if (strncmp(cwd, home, strlen(home)) == 0)
            snprintf(display, sizeof(display), "$H%s", cwd + strlen(home));
        else
            snprintf(display, sizeof(display), "%s", cwd);

        time_t now = time(NULL);
        char timestr[64];
        struct tm *tm_info = localtime(&now);
        strftime(timestr, sizeof(timestr), "%a %b %d %H:%M:%S", tm_info);

        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int cols = w.ws_col;

        int spaces1 = (cols - strlen(display) - strlen(elapsed_str) - strlen(timestr)) / 2;
        if (spaces1 < 1) spaces1 = 1;
        int spaces2 = cols - strlen(display) - spaces1 - strlen(elapsed_str) - strlen(timestr);
        if (spaces2 < 1) spaces2 = 1;

        printf(GREEN "%s" RESET, display);
        for (int s = 0; s < spaces1; s++) printf(" ");
        printf(GREEN "%s" RESET, elapsed_str);
        for (int s = 0; s < spaces2; s++) printf(" ");
        printf(GREEN "%s\n" RESET, timestr);

        char prompt[64];
        if (last_status == 0) snprintf(prompt, sizeof(prompt), GREEN "[%d]OJC> " RESET, last_status);
        else                  snprintf(prompt, sizeof(prompt), RED   "[%d]OJC> " RESET, last_status);

        input = readline(prompt);

        if (!input) {
            printf("just use 'exit' to leave OJC-sh!\n");
            write_history(histfile);
            continue;
        }

        if (*input) add_history(input);

        char *parts[64];
        int n = split_on_delim(input, parts, 64);
        for (int i = 0; i < n; i++) {
            char *cmd = parts[i];
            while (*cmd == ' ') cmd++;
            if (*cmd) run_command(cmd);
        }

        free(input);
    }

    write_history(histfile);
    return 0;
}
