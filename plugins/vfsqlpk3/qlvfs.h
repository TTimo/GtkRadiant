#define MAX_FILE_HANDLES 8192  //64   // qcommon.h
static FILE *xored_open_files[MAX_FILE_HANDLES];
static int xor_max_pos = 0;

FILE* fopen_ql (const char* filename, const char* mode);
size_t fread_ql (void *ptr, size_t size, size_t nmeb, FILE *stream);
int fclose_ql (FILE* stream);