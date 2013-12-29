#include "heap.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void heap_init(heap_t* h)
{
	h->size  = 0;
	h->avail = 0;
	h->tree  = NULL;
}

void heap_exit(heap_t* h)
{
	free(h->tree);
}

static inline int max(int a, int b)
{
	return a > b ? a : b;
}
static inline size_t parent(size_t i)
{
	return max(0, ((ssize_t)i+1)/2-1);
}
static inline size_t left(size_t i)
{
	return 2*i+1;
}
static inline size_t right(size_t i)
{
	return 2*i+2;
}
static inline void xchg(heap_t* h, size_t i, size_t j)
{
	hnode_t tmp;
	memcpy(&tmp,        &h->tree[i], sizeof(hnode_t));
	memcpy(&h->tree[i], &h->tree[j], sizeof(hnode_t));
	memcpy(&h->tree[j], &tmp,        sizeof(hnode_t));
}
static void bubbleUp(heap_t* h, size_t i)
{
	size_t p = parent(i);
	while (h->tree[i].idx < h->tree[p].idx)
	{
		xchg(h, i, p);
		i = p;
		p = parent(i);
	}
}
static void sinkDown(heap_t* h, size_t i)
{
	size_t l = left(i);
	size_t r = right(i);

	size_t next = i;
	if (l < h->size && h->tree[l].idx < h->tree[next].idx)
		next = l;
	if (r < h->size && h->tree[r].idx < h->tree[next].idx)
		next = r;

	if (next != i)
	{
		xchg(h, i, next);
		sinkDown(h, next);
	}
}

void heap_insert(heap_t* h, float idx, void* data)
{
	if (h->size == h->avail)
	{
		h->avail = h->avail ? 2*h->avail : 1;
		h->tree = (hnode_t*) realloc(h->tree, sizeof(hnode_t)*h->avail);
		assert(h->tree != NULL);
	}

	size_t i = h->size++;
	h->tree[i] = (hnode_t){idx,data};
	bubbleUp(h, i);
}

void* heap_remove(heap_t* h)
{
	if (h->size == 0)
		return NULL;

	void* ret = h->tree[0].data;
	xchg(h, 0, --h->size);
	sinkDown(h, 0);
	return ret;
}
