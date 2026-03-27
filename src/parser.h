#ifndef PARSER_H
#define PARSER_H

void expand_vars(char *input, char *output, int size);
void expand_tilde(char *input, char *output, int size);
int parse_input(char *input, char **args);
int split_on_delim(char *input, char **parts, int max);

#endif
