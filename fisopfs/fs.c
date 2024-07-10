#include "fs.h"
#include "utils.h"
#include "entry.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdio.h>

#define READCOEF 2

static struct entry *read_entry(int fd);
static void write_entry(int fd, struct entry *entry);

// Reads fd and makes a `struct metadata`.
static struct metadata
read_metadata(int fd)
{
	struct metadata mdata;

	// Read create, modified and accessed times.
	if (read(fd, &mdata.ctime, sizeof(time_t)) == -1)
		perror("Error reading ctime");

	if (read(fd, &mdata.mtime, sizeof(time_t)) == -1)
		perror("Error reading mtime");

	if (read(fd, &mdata.atime, sizeof(time_t)) == -1)
		perror("Error reading atime");

	// Read ids.
	if (read(fd, &mdata.uid, sizeof(uid_t)) == -1)
		perror("Error reading uid");

	if (read(fd, &mdata.gid, sizeof(gid_t)) == -1)
		perror("Error reading gid");

	// Read mode.
	size_t size = sizeof(mdata.mode);
	if (read(fd, &mdata.mode, size) == -1)
		perror("Error reading mode");

	return mdata;
}

// Writes the data in a `struct metadata` in fd.
static void
write_metadata(int fd, struct metadata *mdata)
{
	// Writ create, modified and accessed times.
	if (write(fd, &mdata->ctime, sizeof(time_t)) == -1)
		perror("Error writing ctime");

	if (write(fd, &mdata->mtime, sizeof(time_t)) == -1)
		perror("Error writing mtime");

	if (write(fd, &mdata->atime, sizeof(time_t)) == -1)
		perror("Error writing atime");

	// Write ids.
	if (write(fd, &mdata->uid, sizeof(uid_t)) == -1)
		perror("Error writing uid");

	if (write(fd, &mdata->gid, sizeof(gid_t)) == -1)
		perror("Error writing gid");

	// Write mode.
	size_t size = sizeof(mdata->mode);
	if (write(fd, &mdata->mode, size) == -1)
		perror("Error writing mode");
}

// Reads fd and maks a `struct file`.
static void
read_file(int fd, struct file *file)
{
	file->capacity = READCOEF * file->len;
	size_t size = file->capacity * sizeof(char);
	file->data = xmalloc(size);

	if (read(fd, file->data, file->len) == -1)
		perror("Error reading file data");
}

// Writes the `struct file` specific fields in fd.
static void
write_file(int fd, struct file *file)
{
	if (write(fd, file->data, file->len) == -1)
		perror("Error writing file data");
}

// Reads fd and makes a `struct dir`.
static void
read_dir(int fd, struct dir *dir)
{
	dir->capacity = READCOEF * dir->len;
	size_t size = dir->capacity * sizeof(void *);
	dir->entries = xmalloc(size);

	for (size_t i = 0; i < dir->len; i++) {
		dir->entries[i] = read_entry(fd);
	}
}

// Writes the `struct dir` specific fields in fd.
static void
write_dir(int fd, struct dir *dir)
{
	for (size_t i = 0; i < dir->len; i++) {
		write_entry(fd, dir->entries[i]);
	}
}

static struct entry *
read_entry(int fd)
{
	struct entry entry;

	// Read name.
	if (read(fd, &entry.name, sizeof(entry.name)) == -1)
		perror("Error reading entry name");

	// Read metadata.
	entry.mdata = read_metadata(fd);

	// Read type.
	if (read(fd, &entry.type, sizeof(entry.type)) == -1)
		perror("Error reading entry type");

	// Read len.
	if (read(fd, &entry.len, sizeof(entry.len)) == -1)
		perror("Error reading entry len");

	struct entry *ret;
	switch (entry.type) {
	case E_FILE:
		ret = (struct entry *) xmalloc(sizeof(struct file));
		*ret = entry;
		read_file(fd, (struct file *) ret);
		return ret;

	case E_DIR:
		ret = (struct entry *) xmalloc(sizeof(struct dir));
		*ret = entry;
		read_dir(fd, (struct dir *) ret);
		return ret;
	}

	perror("Invalid value for entry type");
	return NULL;
}

// Writes the `struct entry` fields in fd.
static void
write_entry(int fd, struct entry *entry)
{
	// Write name.
	size_t size = sizeof(entry->name);
	if (write(fd, entry->name, size) == -1)
		perror("Error writing entry name");

	// Write metadata.
	write_metadata(fd, &entry->mdata);

	// Write type.
	size = sizeof(entry->type);
	if (write(fd, &entry->type, size) == -1)
		perror("Error writing entry type");

	// Write len.
	size = sizeof(entry->len);
	if (write(fd, &entry->len, size) == -1)
		perror("Error writing entry len");

	// Write type specific data.
	switch (entry->type) {
	case E_FILE:
		write_file(fd, (struct file *) entry);
		break;

	case E_DIR:
		write_dir(fd, (struct dir *) entry);
		break;
	}
}

// Initializes the file sysmtem structure
// through a file descriptor.
struct fs
fs_from(int fd)
{
	struct fs fs;

	// Read size.
	if (read(fd, &fs.size, sizeof(fs.size)) == -1)
		perror("Error reading fs size");

	fs.root = (struct dir *) read_entry(fd);
	return fs;
}

// Saves the contents of the file system in a file
// descriptor also freeing the memory it's holding.
void
fs_dump(struct fs fs, int fd)
{
	// Write size.
	if (write(fd, &fs.size, sizeof(fs.size)) == -1)
		perror("Error writing fs size");

	write_entry(fd, (struct entry *) fs.root);
}

// Creates an empty file system.
struct fs
fs_init()
{
	return (struct fs){
		.size = 0,
		.root = create_dir("/", (mode_t) 0755),
	};
}

// Inserts a file in the fs.
int
fs_touch(struct fs *fs, const char *path, mode_t mode)
{
	struct dir *parent = get_parent(*fs, path);
	if (!parent)
		return 1;

	size_t len = 0;
	char **split = split_path(path, &len);

	struct dir *dir = (struct dir *) parent;
	struct file *new_file = create_file(split[len - 1], mode);
	int added = dir_push(dir, (struct entry *) new_file);
	fs->size += added;

	free_split_path(split);
	return 0;
}

// Adds a directory to the fs.
int
fs_mkdir(struct fs *fs, const char *path, mode_t mode)
{
	struct dir *parent = get_parent(*fs, path);
	if (!parent)
		return 1;

	size_t len = 0;
	char **split = split_path(path, &len);

	struct dir *new_dir = create_dir(split[len - 1], mode);
	int added = dir_push(parent, (struct entry *) new_dir);
	fs->size += added;

	free_split_path(split);
	return 0;
}

// Template function for rm and rmdir.
static int
__fs_rm(struct fs *fs, const char *path, enum entrytype type)
{
	struct dir *parent = get_parent(*fs, path);
	if (!parent)
		return 1;

	size_t len = 0;
	char **split = split_path(path, &len);

	int removed = dir_remove(parent, split[len - 1], type);
	fs->size -= removed;

	free_split_path(split);
	return 0;
}

// Removes a file  from the fs.
int
fs_rm(struct fs *fs, const char *path)
{
	if (*path != '/' || fs->size == 0)
		return 0;

	return __fs_rm(fs, path, E_FILE);
}

// Removes a directory from the fs.
int
fs_rmdir(struct fs *fs, const char *path)
{
	if (*path != '/' || fs->size == 0)
		return 0;

	return __fs_rm(fs, path, E_DIR);
}

// Searches for an entry in the fs from a path.
struct entry *
fs_get(struct fs fs, const char *path)
{
	return get_entry(fs, path);
}

// Prints an entry.
static void
entry_print(struct entry *entry, int depth)
{
	printf("%*s%s\n", depth * 4, "", entry->name);

	if (entry->type == E_DIR) {
		struct dir *dir = (struct dir *) entry;
		for (size_t i = 0; i < dir->len; i++) {
			entry_print(dir->entries[i], depth + 1);
		}
	}
}

// Prints the contents of the fs.
void
fs_print(struct fs fs)
{
	puts("==================================");
	printf("FS size: %zu\n", fs.size);
	entry_print((struct entry *) fs.root, 0);
	puts("==================================");
}

// Frees the memory allocated by the fs.
void
fs_free(struct fs fs)
{
	entry_free((struct entry *) fs.root);
}
