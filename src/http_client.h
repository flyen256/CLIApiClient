#ifndef stdio
#include "stdio.h"
#endif

struct Memory {
	char *memory;
	size_t size;
};

typedef struct Memory Memory;

Memory http_get(const char *url);
Memory http_post_json(const char *url, const char *json);
void free_memory(Memory *mem); 
