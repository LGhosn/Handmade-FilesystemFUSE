#ifndef __ENTRY_H__
#define __ENTRY_H__

#include "fs.h"

struct file *create_file(const char *name, mode_t mode);
struct dir *create_dir(const char *name, mode_t mode);

int dir_push(struct dir *parent, struct entry *entry);
int dir_remove(struct dir *dir, const char *name, enum entrytype type);

struct entry *get_entry(struct fs fs, const char *path);
struct dir *get_parent(struct fs fs, const char *path);

int entry_free(struct entry *entry);

void update_times(struct entry *entry);
void update_ctime(struct entry *entry);
void update_atime(struct entry *entry);
void update_mtime(struct entry *entry);

#endif  // __ENTRY_H__
