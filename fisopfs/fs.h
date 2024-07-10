#ifndef __FS_H__
#define __FS_H__
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#define NAMELEN 255
#define OWNERLEN NAMELEN

enum entrytype {
	E_FILE,
	E_DIR,
};

struct fs {
	size_t size;
	struct dir *root;
};

struct metadata {
	time_t ctime;  // changes in the inode.
	time_t mtime;  // changes in the file content.
	time_t atime;  // accessed.
	uid_t uid;
	gid_t gid;
	mode_t mode;
};

// Shared fields between
// `struct file` and `struct dir`.
struct entry {
	char name[NAMELEN];
	struct metadata mdata;
	enum entrytype type;
	size_t len;
};

struct file {
	char name[NAMELEN];
	struct metadata mdata;
	enum entrytype type;
	size_t len;
	// File specific.
	char *data;
	// Not dumpeable.
	size_t capacity;
};

struct dir {
	char name[NAMELEN];
	struct metadata mdata;
	enum entrytype type;
	size_t len;
	// Dir specific.
	struct entry **entries;
	// Not dumpeable.
	size_t capacity;
};

struct fs fs_init();
struct fs fs_from(int fd);
void fs_dump(struct fs fs, int fd);

int fs_touch(struct fs *fs, const char *path, mode_t mode);
int fs_mkdir(struct fs *fs, const char *path, mode_t mode);
int fs_rm(struct fs *fs, const char *path);
int fs_rmdir(struct fs *fs, const char *path);

struct entry *fs_get(struct fs fs, const char *path);
void fs_print(struct fs fs);
void fs_free(struct fs fs);

#endif  // __FS_H__
