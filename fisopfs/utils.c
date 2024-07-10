#include "utils.h"
#include <stdio.h>
#include <string.h>

void *
xmalloc(size_t s)
{
	void *ptr = malloc(s);
	if (!ptr) {
		perror("alloc error");
		exit(1);
	}

	return ptr;
}

void *
xcalloc(size_t q, size_t s)
{
	void *ptr = calloc(q, s);
	if (!ptr) {
		perror("alloc error");
		exit(1);
	}

	return ptr;
}

// Counts the number of times a character appears in a string.
size_t
strcount(const char *str, const char c)
{
	size_t len = strlen(str);
	size_t count = 0;

	for (size_t i = 0; i < len; i++) {
		if (str[i] == c) {
			count++;
		}
	}

	return count;
}

// Splits path into an array of strings, saves the length of the
// array in len. For example:
// "/home/path/to/file.txt" -> ["home", "path", "to", "file.txt"].
char **
split_path(const char *path, size_t *len)
{
	if (*path != '/')
		return NULL;

	char *copy = xmalloc(strlen(path) + 1);
	strcpy(copy, path);

	size_t count = strcount(path, '/');
	*len = count;

	char **split = xmalloc(sizeof(char *) * count);

	lsplit(&copy, '/');
	for (size_t i = 0; i < count; i++) {
		split[i] = lsplit(&copy, '/');
	}

	return split;
}

// Frees the memory used by a split path.
void
free_split_path(char **split_path)
{
	if (!split_path)
		return;

	// split_path[0] - 1 es el path original.
	free(split_path[0] - 1);
	free(split_path);
}

// Splits a string in two at the first occurrence of sep.
char *
lsplit(char **text, const char sep)
{
	char *ptr = *text;
	char *new = *text;

	while (*new != sep &&*new != '\0') {
		new ++;
	}

	if (*new == '\0') {
		*text = new;

	} else if (*new == sep) {
		*new = '\0';
		*text = new + 1;
	}

	return ptr;
}

// Splits a string in two at the last occurrence of sep.
char *
rsplit(char *text, const char sep)
{
	size_t len = strlen(text);
	char *new = text + len;

	while (*new != sep && --len > 0) {
		new --;
	}

	*new = '\0';
	new ++;
	return new;
}
