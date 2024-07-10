#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>

void *xmalloc(size_t s);
void *xcalloc(size_t q, size_t s);
size_t strcount(const char *str, const char c);
char *lsplit(char **text, const char sep);
char *rsplit(char *text, const char sep);
char **split_path(const char *path, size_t *len);
void free_split_path(char **split_path);

#endif  // __UTILS_H__
