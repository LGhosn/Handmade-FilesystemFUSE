#include "entry.h"
#include "utils.h"
#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void
test_split_path(const char *path, char *expected[])
{
	size_t len = 0;
	char **split = split_path(path, &len);

	for (size_t i = 0; i < len; i++) {
		if (strcmp(split[i], expected[i]) != 0) {
			printf("Error: split_path() failed.\n");
		}
	}

	free_split_path(split);
}

void
execute_test_split_path()
{
	test_split_path("/home/path/to/file.txt",
	                (char *[]){ "home", "path", "to", "file.txt" });
	test_split_path("/path", (char *[]){ "path" });
	test_split_path("/", (char *[]){ "" });
}

void
test_strcount(const char *str, const char c, size_t expected)
{
	size_t count = strcount(str, c);

	if (count != expected) {
		printf("Error: strcount() failed, expected %zu, got %zu.\n",
		       expected,
		       count);
	}
}

void
execute_test_strcount()
{
	test_strcount("/home/path/to/file.txt", '/', 4);
	test_strcount("/path", '/', 1);
	test_strcount("/", '/', 1);
	test_strcount("", '/', 0);
}

void
execute_fs_print()
{
	struct fs fs = fs_init();

	fs_touch(&fs, "/README.md", 0644);
	fs_mkdir(&fs, "/src", 0755);
	fs_touch(&fs, "/src/hello_world.c", 0644);
	fs_touch(&fs, "/src/o1_sched.c", 0644);
	fs_mkdir(&fs, "/src/subfolder", 0755);
	fs_touch(&fs, "/src/subfolder/game_of_life.c", 0644);
	fs_mkdir(&fs, "/ai", 0755);
	fs_touch(&fs, "/ai/chatgpt.bf", 0644);
	fs_print(fs);

	// fs_rm(&fs, "/src/subfolder/game_of_life.c");
	// fs_rmdir(&fs, "/src/subfolder");
	// fs_rm(&fs, "/src/o1_sched.c");
	// fs_rm(&fs, "/src/hello_world.c");
	fs_rmdir(&fs, "/src");
	fs_print(fs);

	fs_rmdir(&fs, "/ai");
	fs_print(fs);

	fs_rm(&fs, "/README.md");
	fs_print(fs);

	fs_free(fs);
}

void
execute_read_write()
{
}

void
execute_stat()
{
	struct fs fs = fs_init();

	fs_touch(&fs, "/README.md", 0644);
	fs_mkdir(&fs, "/src", 0755);

	struct entry *entry = get_entry(fs, "/src");
	struct metadata mdata = entry->mdata;

	printf("File: %s\n", entry->name);
	printf("Size: %zu\n", entry->len);
	printf("Mode: %o\n", mdata.mode);
	printf("UID: %u\n", mdata.uid);
	printf("GID: %u\n", mdata.gid);
	printf("Access time: %ld\n", mdata.atime);
	printf("Modify time: %ld\n", mdata.mtime);
	printf("Change time: %ld\n\n\n", mdata.ctime);

	puts("Sleeping for 1 second...");
	sleep(1);

	// Agregamos mas cosas al directorio.
	fs_mkdir(&fs, "/src/subfolder", 0755);
	fs_touch(&fs, "/src/subfolder/hello_world.c", 0644);
	fs_touch(&fs, "/src/subfolder/o1_sched.c", 0644);

	mdata = entry->mdata;

	printf("File: %s\n", entry->name);
	printf("Size: %zu\n", entry->len);
	printf("Mode: %o\n", mdata.mode);
	printf("UID: %u\n", mdata.uid);
	printf("GID: %u\n", mdata.gid);
	printf("Access time: %ld\n", mdata.atime);
	printf("Modify time: %ld\n", mdata.mtime);
	printf("Change time: %ld\n", mdata.ctime);

	fs_free(fs);
}

int
main()
{
	execute_test_split_path();
	execute_test_strcount();
	// execute_fs_print();
	execute_read_write();
	execute_stat();
	return 0;
}