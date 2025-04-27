#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

size_t my_strlen(const char *str);
void my_strcpy(char *dest, const char *src);
void my_strcat(char *dest, const char *src);
int my_strchr(const char *str, char ch);
int my_strncmp(const char *str1, const char *str2, size_t n);

#endif
