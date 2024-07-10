#include "entry.h"
#include "utils.h"
#include "fs.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define DIRINIT 10

void
update_ctime(struct entry *entry)
{
	entry->mdata.ctime = time(NULL);
}

void
update_mtime(struct entry *entry)
{
	entry->mdata.mtime = time(NULL);
}

void
update_atime(struct entry *entry)
{
	entry->mdata.atime = time(NULL);
}


void
update_times(struct entry *entry)
{
	time_t now = time(NULL);
	entry->mdata.ctime = now;
	entry->mdata.mtime = now;
	entry->mdata.atime = now;
}

// Frees the memory used by a `struct entry`.
// Returns the number of entries freed.
int
entry_free(struct entry *entry)
{
	struct file *f = (struct file *) entry;
	struct dir *d = (struct dir *) entry;

	int freed = 0;
	switch (entry->type) {
	case E_FILE:
		free(f->data);
		break;

	case E_DIR:
		for (size_t i = 0; i < d->len; i++) {
			freed += entry_free(d->entries[i]);
		}

		free(d->entries);
		break;
	}

	free(entry);
	return freed + 1;
}

// Creates a new metadata structure with the given mode.
static struct metadata
create_mdata(mode_t mode)
{
	time_t now = time(NULL);
	return (struct metadata){
		.ctime = now,
		.mtime = now,
		.atime = now,
		.gid = getgid(),
		.uid = getuid(),
		.mode = mode,
	};
}

// Creates a new directory structure with the given name
// and an initial capacity for it's entries array of DIRINIT.
struct dir *
create_dir(const char *name, mode_t mode)
{
	struct dir *dir = xmalloc(sizeof(struct dir));

	// Generic fields.
	strncpy(dir->name, name, NAMELEN - 1);
	dir->mdata = create_mdata(mode);
	dir->type = E_DIR;
	dir->len = 0;

	// Not dumpeable.
	dir->capacity = DIRINIT;

	// Dir specific.
	dir->entries = xcalloc(dir->capacity, sizeof(void *));
	return dir;
}

// Creates a new file structure with the given name.
// Data buffer is NULL at initialization and has to
// be initialized with the desired size.
struct file *
create_file(const char *name, mode_t mode)
{
	struct file *file = xmalloc(sizeof(struct file));

	// Generic fields.
	strncpy(file->name, name, NAMELEN - 1);
	file->mdata = create_mdata(mode);
	file->type = E_FILE;
	file->len = 0;

	// Not dumpeable.
	file->capacity = 0;

	// File specific.
	file->data = NULL;
	return file;
}

// Reallocates dir entries array by doubling it's capacity.
static void
dir_expand(struct dir *dir)
{
	size_t new_cap = dir->capacity * 2;
	dir->entries = realloc(dir->entries, new_cap);
}

// Appends a new entry in the entries array
// of a directory. Checks for resizing.
int
dir_push(struct dir *parent, struct entry *entry)
{
	if (!parent)
		return 0;

	if (parent->capacity == parent->len)
		dir_expand(parent);

	parent->entries[parent->len++] = entry;

	// Update parent metadata.
	update_times((struct entry *) parent);
	return 1;
}

// Removes an entry from a directory. If the entry is a directory,
// recursively removes all it's children. Returns the total number
// of entries removed.
int
dir_remove(struct dir *dir, const char *name, enum entrytype type)
{
	update_atime((struct entry *) dir);
	// Iterate over the entries of the directory.
	for (size_t i = 0; i < dir->len; i++) {
		update_atime(dir->entries[i]);
		// If the name matches, check if the type matches.
		if (strcmp(dir->entries[i]->name, name) == 0) {
			if (dir->entries[i]->type != type) {
				break;
			}

			// Free the entry.
			int removed = entry_free(dir->entries[i]);
			dir->len--;

			// Shift entries to the left.
			for (size_t j = i; j < dir->len; j++) {
				dir->entries[j] = dir->entries[j + 1];
			}

			update_ctime((struct entry *) dir);
			update_mtime((struct entry *) dir);
			// entry_free() counted the
			// entry we just removed.
			return removed;
		}
	}

	return 0;
}

// Searches for an entry in the fs from an entry and a split path.
static struct entry *
__get_entry(struct dir *dir, char **split, size_t pos, size_t len)
{
	if (pos == len) {
		return (struct entry *) dir;
	}

	for (size_t i = 0; i < dir->len; i++) {
		struct entry *child = dir->entries[i];
		// Update access time.
		update_atime(child);
		if (strcmp(child->name, split[pos]) == 0) {
			if (pos == len - 1) {
				return child;
			}

			if (child->type != E_DIR) {
				break;
			}

			return __get_entry((struct dir *) child, split, pos + 1, len);
		}
	}

	return NULL;
}

// Searches for an entry in the fs from a split path.
static struct entry *
get_entry_from(struct fs fs, char **split, size_t len)
{
	update_atime((struct entry *) fs.root);
	return __get_entry(fs.root, split, (size_t) 0, len);
}

// Searches for an entry in the fs from a path.
struct entry *
get_entry(struct fs fs, const char *path)
{
	if (strcmp(path, "/") == 0) {
		return (struct entry *) fs.root;
	}

	size_t len = 0;
	char **split = split_path(path, &len);

	struct entry *ret = get_entry_from(fs, split, len);
	free_split_path(split);
	return ret;
}

// Returns the parent of the given path.
// If the path is the root, returns NULL.
// "/a/b/c" -> struct entry *b
struct dir *
get_parent(struct fs fs, const char *path)
{
	size_t len = 0;
	char **split = split_path(path, &len);

	if (len == 0) {
		free_split_path(split);
		return NULL;
	}

	struct entry *ret = get_entry_from(fs, split, len - 1);
	free_split_path(split);
	return (struct dir *) ret;
}
