/*\
 *  Voronoi diagram by Fortune's algorithm
 *  Copyright (C) 2013-2014 Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

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
