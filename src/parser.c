#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "parser.h"

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

void expand_tilde(char *input, char *output, int size) {
  if (input[0] == '~') {
    char *home = getenv("HOME");
    snprintf(output, size, "%s%s", home, input + 1);
  } else {
      strncpy(output, input, size);
      output[size - 1] = '\0';
   }
}

static char _pool[8192];

int parse_input(char *input, char **args) {
  int i = 0;
  int pool = 0;
  char *p = input;

  while (*p) {
    while (*p == ' ' || *p == '\t')
      p++;
    if (!*p)
      break;

  args[i++] = _pool + pool;

      // char *dst = p;
      while (*p && *p != ' ' && *p != '\t') {
        if (*p == '\'') {
          p++;
          while (*p && *p != '\'')
            _pool[pool++] = *p++;
          if (*p == '\'')
            p++;
        }
        else if (*p == '"') {
          p++;
          while (*p && *p != '"') {
            if (*p == '\\') {
              char nx = *(p + 1);
              if (nx == '"' ||
                  nx == '\\' ||
                  nx == '$' ||
                  nx == '`' ||
                  nx == '\n') {
                p++;
              }
            }
            _pool[pool++] = *p++;
          }
          if (*p == '"')
            p++;
        } else if (*p == '\\') {
          p++;
          if (*p) _pool[pool++] = *p++;
        } else {
          _pool[pool++] = *p++;
        }
      }
      _pool[pool++] = '\0';
  }

  args[i] = NULL;
  return i;
}

int split_on_delim(char *input, char **parts, int max) {
  int count = 0;
  char *p = input;
  char *start =p;
  int in_signal = 0, in_double =0;

  while (*p && count < max - 1) {
    if (*p == '\'' && !in_double)
      in_signal = !in_signal;
    else if (*p == '"' && !in_signal)
      in_double = !in_double;
    else if (*p == ';' && !in_signal && !in_double) {
      *p = '\0';
      parts[count++] = start;
      start = p + 1;
    }
    p++;
  }
  parts[count++] = start;
  parts[count] = NULL;
  return count;
}

