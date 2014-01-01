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

#include "voronoi.h"

#include <string.h>

#include "utils.h"

void voronoi_init(voronoi_t* v)
{
	v->done = 0;

	heap_init(&v->events);

	v->a_regions = 0;
	v->n_regions = 0;
	v->regions   = NULL;

	binbeach_init(&v->front);

	v->sweepline  = 0;
	v->n_segments = 0;
	v->a_segments = 0;
	v->segments   = NULL;
}

void voronoi_exit(voronoi_t* v)
{
	free(v->segments);

	binbeach_exit(&v->front);

	event_t* e;
	while ((e = heap_remove(&v->events)) != NULL)
		free(e);
	heap_exit(&v->events);

	for (size_t i = 0; i < v->n_regions; i++)
	{
		region_t* r = v->regions[i];
		free(r->edge_ids);
		free(r);
	}
	free(v->regions);
}

void voronoi_point(voronoi_t* v, point_t p)
{
	if (v->n_regions == v->a_regions)
	{
		v->a_regions = v->a_regions == 0 ? 1 : 2*v->a_regions;
		v->regions = CREALLOC(v->regions, region_t*, v->a_regions);
	}
	region_t* r = CALLOC(region_t, 1);
	*r = (region_t){p, 0, NULL, NULL};
	v->regions[v->n_regions++] = r;

	event_t* e = CALLOC(event_t, 1);
	*e = (event_t){0, 1, r, r->p, NULL};
	heap_insert(&v->events, p.x, e);
}

void voronoi_points(voronoi_t* v, size_t n, point_t* p)
{
	for (; n; p++, n--)
		voronoi_point(v, *p);
}

static void push_circle(voronoi_t* v, bnode_t* n)
{
	if (n->event != NULL)
		n->event->active = 0;
	n->event = NULL;

	// find previous and next arcs
	bnode_t* pa = bnode_prev(n);
	bnode_t* na = bnode_next(n);

	if (pa == NULL || na == NULL)
		return;

	event_t* e = CALLOC(event_t, 1);
	e->active = 1;
	e->r = n->r1;
	double r;
	if (!circle_from3(&e->p, &r, &pa->r1->p, &n->r1->p, &na->r1->p))
	{
		free(e);
		return;
	}

	e->is_circle = 1;
	e->n = n;
	heap_insert(&v->events, e->p.x + r, e);
	n->event = e;
}
static void push_segment(region_t* a, size_t e)
{
	a->edge_ids = CREALLOC(a->edge_ids, size_t, a->n_edges+1);
	a->edge_ids[a->n_edges++] = e;
}
static size_t new_segment(voronoi_t* v, region_t* a, region_t* b)
{
	if (v->n_segments == v->a_segments)
	{
		v->a_segments = v->a_segments == 0 ? 1 : 2*v->a_segments;
		v->segments = CREALLOC(v->segments, segment_t, v->a_segments);
	}
	size_t id = v->n_segments++;
	if (a != NULL) push_segment(a, id);
	if (b != NULL) push_segment(b, id);
	return id;
}
char voronoi_step(voronoi_t* v)
{
	double idx = 0;
	if (v->events.size != 0)
		idx = v->events.tree[0].idx;

	event_t* e = heap_remove(&v->events);
	if (e == NULL)
		return 0;

	if (!e->active)
	{
		free(e);
		return 1;
	}

	v->sweepline = idx;

	if (e->is_circle)
	{
		// current arc
		bnode_t* n = e->n;

		// finish segments at breakpoints
		bnode_t* lb = bnode_right(n);
		bnode_t* rb = bnode_left (n);
		*voronoi_id2point(v, lb->end) = e->p;
		*voronoi_id2point(v, rb->end) = e->p;

		// save previous and next arcs
		bnode_t* pa = bnode_prev(n);
		bnode_t* na = bnode_next(n);

		// remove arc
		n = bnode_remove(n);

		// refresh circle events
		push_circle(v, pa);
		push_circle(v, na);

		// start new segment
		size_t s = new_segment(v, pa->r1, na->r1);
		n->end = 2*s+1;
		*voronoi_id2point(v, 2*s) = e->p;
	}
	else
	{
		bnode_t* n = binbeach_breakAt(&v->front, v->sweepline, e->r);

		if (n->left == NULL)
		{
			free(e);
			return 1;
		}

		// insert events
		push_circle(v, n->left);
		push_circle(v, n->right->right);

		// add segment
		size_t s = new_segment(v, n->r1, e->r);
		n->end        = 2*s;
		n->right->end = 2*s+1;
	}

	free(e);
	return 1;
}
static char inRect(point_t* p)
{
	return
	0 < p->x && p->x < 20 &&
	0 < p->y && p->y < 20 &&
	1;
}

static void finishSegments(voronoi_t* v, bnode_t* n)
{
	if (n->left == NULL)
		return;

	point_t p;
	parabola_intersect(&p, &n->r1->p, &n->r2->p, v->sweepline);
	*voronoi_id2point(v, n->end) = p;

	finishSegments(v, n->left);
	finishSegments(v, n->right);
}
static void voronoi_restrictRegion(voronoi_t* v, region_t* r)
{
	static const segment_t border[4] =
	{
		{{ 0, 0},{ 0,20}},
		{{ 0,20},{20,20}},
		{{20,20},{20, 0}},
		{{20, 0},{ 0, 0}},
	};

	// find two jutting edges
	point_t* a = NULL;
	point_t* b = NULL;
	for (ssize_t j = 0; j < (ssize_t) r->n_edges; j++)
	{
		segment_t* s = voronoi_id2segment(v, r->edge_ids[j]);
		char ak = inRect(&s->a);
		char bk = inRect(&s->b);

		if (!ak && !bk)
		{
			r->n_edges--;
			memmove(r->edge_ids+j, r->edge_ids+j+1, sizeof(size_t)*(r->n_edges-j));
			j--;
		}
		else if (ak != bk)
		{
			point_t* p = !ak ? &s->a : &s->b;
			for (size_t k = 0; k < 4 && !segment_intersect(p, &border[k], s); k++);

			if (a == NULL) a = p;
			else           b = p;
		}
	}

	if (a == NULL || b == NULL)
		return;

	point_t p;
	// handle sides
	if (a->x == b->x || a->y == b->y)
	{
		size_t id = new_segment(v, r, NULL);
		segment_t* s = voronoi_id2segment(v, id);
		s->a = *a;
		s->b = *b;
		return;
	}
	// handle corners (two sides)
	else if ((a->x == 0 || a->x == 20) && (b->y == 0 || b->y == 20))
	{
		p.x = a->x;
		p.y = b->y;
	}
	else if ((a->y == 0 || a->y == 20) && (b->x == 0 || b->x == 20))
	{
		p.x = b->x;
		p.y = a->y;
	}
	// should handle three or four sides
	else
		return;

	// apply common corner correction
	size_t id1 = new_segment(v, r, NULL);
	size_t id2 = new_segment(v, r, NULL);
	segment_t* s1 = voronoi_id2segment(v, id1);
	segment_t* s2 = voronoi_id2segment(v, id2);
	s1->a = *a;
	s1->b = p;
	s2->a = p;
	s2->b = *b;
}
void voronoi_end(voronoi_t* v)
{
	while (voronoi_step(v));

	v->sweepline += 1000;
	finishSegments(v, v->front.root);

	for (size_t i = 0; i < v->n_regions; i++)
		voronoi_restrictRegion(v, v->regions[i]);

	v->done = 1;
}

void voronoi_ptrs(voronoi_t* v)
{
	for (size_t i = 0; i < v->n_regions; i++)
	{
		region_t* r = v->regions[i];
		r->edges = CALLOC(segment_t*, r->n_edges);
		for (size_t j = 0; j < r->n_edges; j++)
		{
			size_t id = r->edge_ids[j];
			segment_t* e = voronoi_id2segment(v, id);
			r->edges[j] = e;
		}
	}
}

inline point_t* voronoi_id2point(voronoi_t* v, size_t id)
{
	segment_t* s = &v->segments[id/2];
	return id % 2 == 0 ? &s->a : &s->b;
}
inline segment_t* voronoi_id2segment(voronoi_t* v, size_t id)
{
	return &v->segments[id];
}
