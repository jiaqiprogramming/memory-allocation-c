
#ifndef __MY_ALLOC_HEADER__
#define __MY_ALLOC_HEADER__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
typedef struct BlockInfo {
	void* ptr;
	size_t size;
	struct BlockInfo* prev, * next;
	bool used;
} BlockInfo;

void myinit(int allocAlg);
void* mymalloc(size_t size);
void myfree(void* ptr);
void* myrealloc(void* ptr, size_t size);
void mycleanup();

#define MEMORY_SIZE (1 * 1024 * 1024)
#define ALIGN 8
#define MAX_MEMORY_BLOCK (MEMORY_SIZE / ALIGN)

#ifdef DEBUG
void showMemoryList();
#endif
#endif