#include <stdint.h>
#include <stdbool.h>

#define FILENAME_SIZE 31

typedef struct LFile LFile;

int INITLLFS();

int load_disk();

int mkdir(char *pathname);

bool rm_dir(char *pathname);

LFile *open_file(char *pathname, char *model);

bool rm_file(char *pathname);

void write_file(void *ptr, size_t size, size_t nmemb, LFile *lfile);

size_t read_file(void *ptr, size_t size, size_t nmemb, LFile *lfile);

void close_file(LFile *lfile);
