#ifndef HEAP_H
#define HEAP_H

typedef struct hnode hnode_t;
typedef struct heap  heap_t;

#include <sys/types.h>

struct hnode
{
	double idx;
	void* data;
};

struct heap
{
	size_t   size;
	size_t   avail;
	hnode_t* tree;
};

void heap_init(heap_t* h);
void heap_exit(heap_t* h);

void  heap_insert(heap_t* h, double idx, void* data);
void* heap_remove(heap_t* h);

#endif
