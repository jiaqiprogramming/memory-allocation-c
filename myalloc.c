#include "myalloc.h"

BlockInfo* Pool;
BlockInfo* Mem;
BlockInfo* last;
void* heapSpace;
int algorithm;

void initMemoryPool() {
	Pool = malloc(sizeof(BlockInfo));
	Pool->prev = NULL;
	Pool->next = NULL;
	for (int i = 0;i <= MAX_MEMORY_BLOCK;i++) {
		BlockInfo* tmp = malloc(sizeof(BlockInfo));
		tmp->next = Pool;
		Pool->prev = tmp;
		Pool = tmp;
	}
}

BlockInfo* allocMeta() {
	BlockInfo* ans = Pool;
	Pool = Pool->next;
	return ans;
}

void freeMeta(BlockInfo* x) {
	x->next = Pool;
	Pool->prev = x;
	Pool = x;
}

size_t align(size_t req) {
	return (req + 7) / 8 * 8;
}

void showMemoryList() {
	BlockInfo* cur = Mem;
	puts("=======Memory Dump=======");
	while (cur) {
		size_t sz = cur->size;
		int unit = 0;
		while (sz >= 1024) {
			sz /= 1024;
			unit++;
		}
		printf("%p %lu%c ", cur->ptr, sz, "BKMG"[unit]);
		if (cur->used) {
			puts("[Allocated]");
		}
		else {
			puts("[Free]");
		}
		cur = cur->next;
	}
}

BlockInfo* merge(BlockInfo* x) {
	while (x->prev && !x->prev->used) {
		x->prev->size += x->size;
		x->prev->next = x->next;
		if (x->next)x->next->prev = x->prev;
		BlockInfo* tmp = x->prev;
		if (algorithm == 1 && last == x)last = tmp;
		freeMeta(x);
		x = tmp;
	}
	while (x->next && !x->next->used) {
		x->size += x->next->size;
		BlockInfo* tmp = x->next;
		if (algorithm == 1 && last == tmp)last = x;
		x->next = x->next->next;
		if (x->next)x->next->prev = x;
		freeMeta(tmp);
	}
	return x;
}

void myinit(int allocAlg) {
	initMemoryPool();
	Mem = allocMeta();
	Mem->next = NULL;
	Mem->prev = NULL;
	heapSpace = Mem->ptr = malloc(MEMORY_SIZE);
	Mem->size = MEMORY_SIZE;
	Mem->used = false;
	algorithm = allocAlg;
	if (algorithm == 1) {
		last = Mem;
	}
}

void* mymalloc(size_t size) {
	if (size == 0)return NULL;
	size = align(size);
	BlockInfo* startPoint = Mem;
	if (algorithm == 1) {
		startPoint = last->next;
		if (startPoint == NULL)
			startPoint = Mem;
	}
	BlockInfo* decision = NULL;
	BlockInfo* next = startPoint;
	BlockInfo* cur = startPoint;
	do {
		cur = next;
		if (!cur->used && cur->size >= size) {
			if (decision == NULL || (algorithm == 2 && decision->size > cur->size)) {
				decision = cur;
			}
		}
		next = cur->next;
		if (next == NULL)next = Mem;
	} while (next != startPoint);
	if (decision != NULL) {
		if (decision->size == size) {
			decision->used = true;
			if (algorithm == 1) {
				last = decision->next;
				if (!last) last = Mem;
			}
			return decision->ptr;
		}
		else {
			//split
			BlockInfo* info = allocMeta();
			info->prev = decision;
			info->next = decision->next;
			if (info->next)info->next->prev = info;
			decision->next = info;
			info->size = decision->size - size;
			decision->size = size;
			decision->used = true;
			info->used = false;
			info->ptr = decision->ptr + size;
			if (algorithm == 1) {
				last = info;
			}
			return decision->ptr;
		}
	}
	else return NULL;
}

void myfree(void* ptr) {
	if (!ptr)return;
	for (BlockInfo* x = Mem; x; x = x->next) {
		if (x->ptr == ptr) {
			x->used = false;
			merge(x);
			return;
		}
	}
}
void* myrealloc(void* ptr, size_t size) {
	if (size == 0) {
		myfree(ptr);
		return NULL;
	}
	if (ptr == NULL) {
		return mymalloc(size);
	}
	size = align(size);
	for (BlockInfo* x = Mem; x; x = x->next) {
		if (x->ptr == ptr) {
			if (size == x->size) {
				return ptr;
			}
			if (size < x->size) {
				//shrink
				size_t delta = x->size - size;
				BlockInfo* tmp = allocMeta();
				tmp->size = delta;
				tmp->ptr = x->ptr + size;
				tmp->prev = x;
				tmp->next = x->next;
				if (tmp->next)tmp->next->prev = tmp;
				x->next = tmp;
				x->size = size;
				tmp->used = false;
				merge(tmp);
				return x->ptr;
			}
			// expand mem
			size_t delta = size - x->size;

			if (x->next && !x->next->used && x->next->size >= delta) {
				// merge mem block
				x->size += delta;
				x->next->size -= delta;
				if (x->next->size == 0) {
					// remove from free list
					if (algorithm == 1 && last == x->next) {
						last = x->next->next;
						if (!last)last = Mem;
					}
					BlockInfo* tmp = x->next;
					x->next = tmp->next;
					if (x->next)x->next->prev = x;
					freeMeta(tmp);

				}
				return x->ptr;
			}
			else {
				void* allocated = mymalloc(size);
				if (allocated) {
					memcpy(allocated, ptr, x->size);
					myfree(ptr);
					return allocated;
				}
				else return NULL;
			}
		}
	}
	fprintf(stderr, "%p realloc but not in list\n", ptr);
	exit(-1);
}
void mycleanup() {
	free(heapSpace);
	BlockInfo* next;
	for (BlockInfo* x = Mem;x; x = next) {
		next = x->next;
		free(x);
	}
	for (BlockInfo* x = Pool;x; x = next) {
		next = x->next;
		free(x);
	}
}