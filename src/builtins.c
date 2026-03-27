#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include "shell.h"
#include "builtins.h"

int run_builtin(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        printf("as you like...\n");
        exit(last_status);
    }
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) { chdir(getenv("HOME")); last_status = 0; }
        else {
            if (chdir(args[1]) != 0) { fprintf(stderr, "W: did u type it right? %s\n", args[1]); last_status = 1; }
            else last_status = 0;
        }
        return 1;
    }
    if (strcmp(args[0], "his") == 0) {
        HIST_ENTRY **hist = history_list();
        if (hist)
            for (int h = 0; hist[h]; h++)
                printf("%d %s\n", h + 1, hist[h]->line);
        last_status = 0;
        return 1;
    }
    if (strcmp(args[0], "who?") == 0) {
        printf("        >_ \n OJC // SHELL\n \n [ built from silence ]\n [ executed in control ]\n \n      — ojclicks —\n");
        last_status = 0;
        return 1;
    }
    if (strcmp(args[0], "hello") == 0) {
        printf("hello from OJC-shell a vision for OJclicks os...\n");
        last_status = 0;
        return 1;
    }
    if (strcmp(args[0], "help") == 0) {
        printf("try this commands...\n'who?'\n'hello'\n'about'\n'ojcshfetch'\n'his'\n");
        last_status = 0;
        return 1;
    }
    if (strcmp(args[0], "about") == 0) {
        printf("OJC-shell — a minimal Unix-like shell written in C\n");
        printf("Part of the OJclicks OS journey\n");
        printf("Not here to compete with Bash...\njust here to understand it\n");
        printf("works...\nbreaks...\nteaches...\n");
        printf("If it crashes, it's a lesson.\nIf it works, question it.\n");
        last_status = 0;
        return 1;
    }
    if (strcmp(args[0], "ojcshfetch") == 0) {
        printf("\n"
         "   ██████╗      ██╗ ██████╗      ███████╗██╗  ██╗███████╗██╗     ██╗\n"
         "  ██╔═══██╗     ██║██╔════╝      ██╔════╝██║  ██║██╔════╝██║     ██║\n"
         "  ██║   ██║     ██║██║           ███████╗███████║█████╗  ██║     ██║\n"
         "  ██║   ██║██   ██║██║           ╚════██║██╔══██║██╔══╝  ██║     ██║\n"
         "  ╚██████╔╝╚█████╔╝╚██████╗      ███████║██║  ██║███████╗███████╗███████╗\n"
         "   ╚═════╝  ╚════╝  ╚═════╝      ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝\n"
         "\n                >_  O J C   S H E L L  _<\n\n");
        last_status = 0;
        return 1;
    }
    return 0;
}
