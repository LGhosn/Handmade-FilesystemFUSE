#define FUSE_USE_VERSION 30

#include "entry.h"
#include "fs.h"
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>

#define FSPATH "filesystem.fisopfs"
char fs_path[NAMELEN];

#define MAXENTRIES 128
struct fs fs;

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	struct entry *entry = fs_get(fs, path);
	if (entry == NULL) {
		return -ENOENT;
	}

	st->st_nlink = 0;
	st->st_size = entry->len;
	st->st_ctime = entry->mdata.ctime;
	st->st_mtime = entry->mdata.mtime;
	st->st_atime = entry->mdata.atime;
	st->st_uid = entry->mdata.uid;
	st->st_gid = entry->mdata.gid;
	st->st_mode = entry->mdata.mode |
	              (entry->type == E_DIR ? __S_IFDIR : __S_IFREG);

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por otro directorio, lo buscamos en nuestro fs.
	struct entry *entry = fs_get(fs, path);
	if (!entry || entry->type != E_DIR) {
		return -ENOENT;
	}

	struct dir *dir = (struct dir *) entry;
	for (size_t i = 0; i < dir->len; i++) {
		filler(buffer, dir->entries[i]->name, NULL, 0);
	}

	return 0;
}

// #define MAX_CONTENIDO 100
// static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

size_t
min(size_t a, size_t b)
{
	return a < b ? a : b;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct entry *entry = fs_get(fs, path);
	if (!entry || entry->type != E_FILE) {
		return -ENOENT;
	}

	struct file *file = (struct file *) entry;
	if (offset + size > file->len)
		size = file->len - offset;

	strncpy(buffer, file->data + offset, size);
	return size;
}

static int
fisopfs_createfiles(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_createfiles - path: %s\n", path);
	if (fs.size == MAXENTRIES)
		return -ENOSPC;

	return fs_touch(&fs, path, mode) == 0 ? 0 : -ENOENT;
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");
	int fd = open(fs_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) {
		perror("Error saving fs");
	} else {
		fs_dump(fs, fd);
	}

	fs_free(fs);
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisop_flush - path: %s\n", path);
	int fd = open(fs_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) {
		perror("Error saving fs");
		return 1;
	}

	fs_dump(fs, fd);
	return 0;
}

static int
fisopfs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
	printf("[debug] fisop_fsync - path: %s\n", path);

	return 0;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisop_init\n");
	int fd = open(fs_path, O_RDONLY);
	if (fd == -1) {
		fs = fs_init();
	} else {
		fs = fs_from(fd);
		close(fd);
	}

	return NULL;
}

static int
fisopfs_link(const char *from, const char *to)
{
	printf("[debug] fisop_link - from: %s, to: %s\n", from, to);

	return 0;
}

static int
fisopfs_createdir(const char *path, mode_t mode)
{
	printf("[debug] fisop_createdir - path: %s\n", path);
	if (fs.size == MAXENTRIES)
		return -ENOSPC;

	return fs_mkdir(&fs, path, mode) == 0 ? 0 : -ENOENT;
}

static int
fisopfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("[debug] fisop_mknod - path: %s\n", path);

	return 0;
}

static int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisop_opendir - path: %s\n", path);

	return 0;
}

static int
fisopfs_readlink(const char *path, char *buffer, size_t size)
{
	printf("[debug] fisop_readlink - path: %s\n", path);

	return 0;
}

static int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisop_release - path: %s\n", path);

	return 0;
}

static int
fisopfs_removedir(const char *path)
{
	printf("[debug] fisop_removedir - path: %s\n", path);
	return fs_rmdir(&fs, path);
}

static int
fisopfs_rename(const char *from, const char *to)
{
	printf("[debug] fisop_rename - from: %s, to: %s\n", from, to);

	return 0;
}

static int
fisopfs_stats(const char *path, struct statvfs *stbuf)
{
	printf("[debug] fisop_stats - path: %s\n", path);

	return 0;
}

static int
fisopfs_removefile(const char *path)
{
	printf("[debug] fisop_removefile - path: %s\n", path);
	return fs_rm(&fs, path);
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisop_utimens - path: %s\n", path);

	return 0;
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisop_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct entry *entry = fs_get(fs, path);
	if (!entry || entry->type != E_FILE) {
		return -ENOENT;
	}

	struct file *file = (struct file *) entry;
	if (file->len + size > file->capacity) {
		file->capacity = 2 * (file->len + size);
		file->data = realloc(file->data, file->capacity);
	}

	strncpy(file->data + offset, buffer, size);
	file->len = offset + size;
	file->data[file->len] = '\0';

	// Update metadata.
	update_ctime(entry);
	return size;
}

static int
fisopfs_access(const char *path, int mask)
{
	printf("[debug] fisop_access - path: %s\n", path);
	struct entry *entry = fs_get(fs, path);
	if (!entry) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_chmod(const char *path, mode_t mode)
{
	printf("[debug] fisop_chmod - path: %s\n", path);
	struct entry *entry = fs_get(fs, path);
	if (!entry) {
		return -ENOENT;
	}

	entry->mdata.mode = mode;

	// Update metadata.
	update_ctime(entry);
	return 0;
}

static int
fisopfs_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("[debug] fisop_chown - path: %s\n", path);
	struct entry *entry = fs_get(fs, path);
	if (!entry) {
		return -ENOENT;
	}

	entry->mdata.uid = uid;
	entry->mdata.gid = gid;

	// Update metadata.
	update_ctime(entry);
	return 0;
}

static int
fisopfs_symlink(const char *from, const char *to)
{
	printf("[debug] fisop_symlink - from: %s, to: %s\n", from, to);

	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisop_truncate - path: %s, size: %lu\n", path, size);

	struct entry *entry = fs_get(fs, path);
	if (!entry || entry->type != E_FILE) {
		return -ENOENT;
	}

	struct file *file = (struct file *) entry;
	file->data = realloc(file->data, size);
	file->capacity = size;

	// Update metadata.
	update_ctime(entry);
	update_mtime(entry);
	return 0;
}

static int
fisopfs_setxattr(const char *path,
                 const char *name,
                 const char *value,
                 size_t size,
                 int flags)
{
	printf("[debug] fisop_setxattr - path: %s, name: %s, value: %s, size: "
	       "%lu, flags: %d\n",
	       path,
	       name,
	       value,
	       size,
	       flags);

	return 0;
}

static int
fisopfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
	printf("[debug] fisop_getxattr - path: %s, name: %s, value: %s, size: "
	       "%lu\n",
	       path,
	       name,
	       value,
	       size);

	return 0;
}

static int
fisopfs_listxattr(const char *path, char *list, size_t size)
{
	printf("[debug] fisop_listxattr - path: %s, list: %s, size: %lu\n",
	       path,
	       list,
	       size);

	return 0;
}

static int
fisopfs_removexattr(const char *path, const char *name)
{
	printf("[debug] fisop_removexattr - path: %s, name: %s\n", path, name);

	return 0;
}

static int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisop_releasedir - path: %s\n", path);

	return 0;
}

static int
fisopfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
	printf("[debug] fisop_fsyncdir - path: %s\n", path);

	return 0;
}

static int
fisopfs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock)
{
	printf("[debug] fisop_lock - path: %s\n", path);

	return 0;
}

static int
fisopfs_bmap(const char *path, size_t blocksize, uint64_t *idx)
{
	printf("[debug] fisop_bmap - path: %s\n", path);

	return 0;
}

static int
fisopfs_ioctl(const char *path,
              int cmd,
              void *arg,
              struct fuse_file_info *fi,
              unsigned int flags,
              void *data)
{
	printf("[debug] fisop_ioctl - path: %s\n", path);

	return 0;
}

static int
fisopfs_poll(const char *path,
             struct fuse_file_info *fi,
             struct fuse_pollhandle *ph,
             unsigned *reventsp)
{
	printf("[debug] fisop_poll - path: %s\n", path);

	return 0;
}

static int
fisopfs_flock(const char *path, struct fuse_file_info *fi, int op)
{
	printf("[debug] fisop_flock - path: %s\n", path);

	return 0;
}

static int
fisopfs_fallocate(const char *path,
                  int mode,
                  off_t offset,
                  off_t length,
                  struct fuse_file_info *fi)
{
	printf("[debug] fisop_fallocate - path: %s\n", path);

	return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.access = fisopfs_access,
	.chmod = fisopfs_chmod,
	.chown = fisopfs_chown,
	.create = fisopfs_createfiles,
	.destroy = fisopfs_destroy,
	.flush = fisopfs_flush,
	.fsync = fisopfs_fsync,
	.init = fisopfs_init,
	.link = fisopfs_link,
	.mkdir = fisopfs_createdir,
	.mknod = fisopfs_mknod,
	.opendir = fisopfs_opendir,
	.readlink = fisopfs_readlink,
	.release = fisopfs_release,
	.rmdir = fisopfs_removedir,
	.rename = fisopfs_rename,
	.statfs = fisopfs_stats,
	.unlink = fisopfs_removefile,
	.utimens = fisopfs_utimens,
	.write = fisopfs_write,
	.symlink = fisopfs_symlink,
	.truncate = fisopfs_truncate,
	.setxattr = fisopfs_setxattr,
	.getxattr = fisopfs_getxattr,
	.listxattr = fisopfs_listxattr,
	.removexattr = fisopfs_removexattr,
	.releasedir = fisopfs_releasedir,
	.fsyncdir = fisopfs_fsyncdir,
	.lock = fisopfs_lock,
	.bmap = fisopfs_bmap,
	.ioctl = fisopfs_ioctl,
	.poll = fisopfs_poll,
	.flock = fisopfs_flock,
	.fallocate = fisopfs_fallocate,
};

int
main(int argc, char *argv[])
{
	memset(fs_path, 0, NAMELEN);
	if (argc == 4) {
		strcpy(fs_path, argv[3]);
	} else {
		strcpy(fs_path, FSPATH);
	}

	return fuse_main(argc, argv, &operations, NULL);
}
