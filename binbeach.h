#ifndef BINBEACH_H
#define BINBEACH_H

typedef struct bnode    bnode_t;
typedef struct binbeach binbeach_t;

#include <sys/types.h>

#include "geometry.h"

struct region;
struct event;

// internal nodes are breakpoints
// (two regions, two children, 'end' set)
// leaves are arcs (one region, no child, 'event' set)
struct bnode
{
	struct region* r1;
	struct region* r2;

	bnode_t* left;
	bnode_t* right;
	bnode_t* parent;

	size_t        end;
	struct event* event;
};

struct binbeach
{
	bnode_t* root;
};

void binbeach_init(binbeach_t* b);
void binbeach_exit(binbeach_t* b);

bnode_t* binbeach_breakAt(binbeach_t* b, double sweep, struct region* r);

// bnode_X finds closest ancestor of n for which n is X to
bnode_t* bnode_left (bnode_t* n);
bnode_t* bnode_right(bnode_t* n);

bnode_t* bnode_prev(bnode_t* n);
bnode_t* bnode_next(bnode_t* n);

// remove an arc, return the new breakpoint
bnode_t* bnode_remove(bnode_t* n);

#endif
