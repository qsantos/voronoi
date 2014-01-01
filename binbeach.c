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

#include "binbeach.h"

#include "utils.h"
#include "voronoi.h"

void binbeach_init(binbeach_t* b)
{
	b->root = NULL;
}

static void exit_aux(bnode_t* n)
{
	if (n == NULL)
		return;

	exit_aux(n->left);
	exit_aux(n->right);
	free(n);
}
void binbeach_exit(binbeach_t* b)
{
	exit_aux(b->root);
}

bnode_t* binbeach_breakAt(binbeach_t* b, double sweep, struct region* r)
{
	if (b->root == NULL)
	{
		bnode_t* n = CALLOC(bnode_t, 1);
		*n = (bnode_t){r, NULL, NULL, NULL, NULL, 0, NULL};
		b->root = n;
		return n;
	}

	// find the intersecting arc
	double y = r->p.y;
	bnode_t* n = b->root;
	while (n->r2 != NULL)
	{
		point_t p;
		parabola_intersect(&p, &n->r1->p, &n->r2->p, sweep);
		if (y < p.y)
			n = n->left;
		else
			n = n->right;
	}

	// left leaf (original region)
	bnode_t* ll = CALLOC(bnode_t, 1);
	*ll = (bnode_t){n->r1, NULL, NULL, NULL, n, 0, n->event};
	n->left = ll;

	// new internal node
	bnode_t* ni = CALLOC(bnode_t, 1);

	// middle leaf (new region)
	bnode_t* ml = CALLOC(bnode_t, 1);
	*ml = (bnode_t){r, NULL, NULL, NULL, ni, 0, NULL};

	// right leaf (original region)
	bnode_t* rl = CALLOC(bnode_t, 1);
	*rl = (bnode_t){n->r1, NULL, NULL, NULL, ni, 0, n->event};

	// filling new internal node
	*ni = (bnode_t){r, n->r1, ml, rl, n, 0, NULL};

	// filling old leaf node (now internal)
	n->r2    = r;
	n->left  = ll;
	n->right = ni;

	return n;
}

bnode_t* bnode_left(bnode_t* n)
{
	while (n->parent != NULL && n != n->parent->left)
		n = n->parent;
	return n->parent;
}

bnode_t* bnode_right(bnode_t* n)
{
	while (n->parent != NULL && n != n->parent->right)
		n = n->parent;
	return n->parent;
}

bnode_t* bnode_prev(bnode_t* n)
{
	n = bnode_right(n);
	if (n == NULL)
		return NULL;
	n = n->left;
	while (n->right != NULL)
		n = n->right;
	return n;
}

bnode_t* bnode_next(bnode_t* n)
{
	n = bnode_left(n);
	if (n == NULL)
		return NULL;
	n = n->right;
	while (n->left != NULL)
		n = n->left;
	return n;
}

bnode_t* bnode_remove(bnode_t* n)
{
	bnode_t* p = n->parent;

	bnode_t* s; // sibling
	bnode_t* a; // new breakpoint

	// find sibling and breakpoint
	if (n == p->left)
	{
		s = p->right;
		a = bnode_right(n);
		a->r2 = p->r2;
	}
	else
	{
		s = p->left;
		a = bnode_left(n);
		a->r1 = p->r1;
	}

	// find where to put sibling
	bnode_t* pp = p->parent;
	bnode_t** x = p == pp->left ? &pp->left  : &pp->right;

	*x = s;
	s->parent = pp;

	free(n);
	free(p);

	return a;
}
