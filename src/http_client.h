#ifndef stdio
#include "stdio.h"
#endif

struct Memory {
	char *memory;
	size_t size;
};

typedef struct Memory Memory;

Memory http_get(const char *url);
void free_memory(Memory *mem); 
