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

void vr_diagram_init(vr_diagram_t* v)
{
	v->n_segments = 0;
	v->a_segments = 0;
	v->segments   = NULL;

	v->n_points = 0;
	v->a_points = 0;
	v->points   = NULL;

	v->n_regions = 0;
	v->a_regions = 0;
	v->regions   = NULL;

	heap_init(&v->events);
	vr_binbeach_init(&v->front);
	v->sweepline  = 0;
}

void vr_diagram_exit(vr_diagram_t* v)
{
	vr_binbeach_exit(&v->front);

	vr_event_t* e;
	while ((e = heap_remove(&v->events)) != NULL)
		free(e);
	heap_exit(&v->events);

	for (size_t i = 0; i < v->n_regions; i++)
	{
		vr_region_t* r = v->regions[i];
		free(r->edges);
		free(r);
	}
	free(v->regions);

	for (size_t i = 0; i < v->n_segments; i++)
		free(v->segments[i]);
	free(v->segments);

	for (size_t i = 0; i < v->n_points; i++)
		free(v->points[i]);
	free(v->points);
}

void vr_diagram_point(vr_diagram_t* v, point_t p)
{
	if (v->n_regions == v->a_regions)
	{
		v->a_regions = v->a_regions == 0 ? 1 : 2*v->a_regions;
		v->regions = CREALLOC(v->regions, vr_region_t*, v->a_regions);
	}
	vr_region_t* r = CALLOC(vr_region_t, 1);
	*r = (vr_region_t){p, 0, NULL};
	v->regions[v->n_regions++] = r;

	vr_event_t* e = CALLOC(vr_event_t, 1);
	*e = (vr_event_t){0, 1, r, NULL, NULL};
	heap_insert(&v->events, p.x, e);
}

void vr_diagram_points(vr_diagram_t* v, size_t n, point_t* p)
{
	for (; n; p++, n--)
		vr_diagram_point(v, *p);
}

static void new_point(vr_diagram_t* v, vr_point_t* np)
{
	if (v->n_points == v->a_points)
	{
		v->a_points = v->a_points == 0 ? 1 : 2*v->a_points;
		v->points = CREALLOC(v->points, vr_point_t*, v->a_points);
	}
	v->points[v->n_points++] = np;
}
static void push_circle(vr_diagram_t* v, vr_bnode_t* n)
{
	if (n->event != NULL)
		n->event->active = 0;
	n->event = NULL;

	// find previous and next arcs
	vr_bnode_t* pa = vr_bnode_prev(n);
	vr_bnode_t* na = vr_bnode_next(n);

	if (pa == NULL || na == NULL)
		return;

	vr_event_t* e = CALLOC(vr_event_t, 1);
	e->active = 1;
	e->r = n->r1;
	e->p = CALLOC(vr_point_t, 1);
	double r;
	if (!circle_from3(&e->p->p, &r, &pa->r1->p, &n->r1->p, &na->r1->p))
	{
		free(e->p);
		free(e);
		return;
	}

	new_point(v, e->p);
	e->is_circle = 1;
	e->n = n;
	heap_insert(&v->events, e->p->p.x + r, e);
	n->event = e;
}

static void push_edge(vr_region_t* a, vr_segment_t* s)
{
	a->edges = CREALLOC(a->edges, vr_segment_t*, a->n_edges+1);
	a->edges[a->n_edges++] = s;
}
static vr_segment_t* new_segment(vr_diagram_t* v, vr_region_t* a, vr_region_t* b)
{
	if (v->n_segments == v->a_segments)
	{
		v->a_segments = v->a_segments == 0 ? 1 : 2*v->a_segments;
		v->segments = CREALLOC(v->segments, vr_segment_t*, v->a_segments);
	}

	vr_segment_t* s = CALLOC(vr_segment_t, 1);
	*s = (vr_segment_t){{NULL, NULL}, a, b};
	if (a != NULL) push_edge(a, s);
	if (b != NULL) push_edge(b, s);
	return s;
}
char vr_diagram_step(vr_diagram_t* v)
{
	double idx = 0;
	if (v->events.size != 0)
		idx = v->events.tree[0].idx;

	vr_event_t* e = heap_remove(&v->events);
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
		vr_bnode_t* n = e->n;

		// finish segments at breakpoints
		vr_bnode_t* lb = vr_bnode_right(n);
		vr_bnode_t* rb = vr_bnode_left (n);
		*lb->end = e->p;
		*rb->end = e->p;

		// save previous and next arcs
		vr_bnode_t* pa = vr_bnode_prev(n);
		vr_bnode_t* na = vr_bnode_next(n);

		// remove arc
		n = vr_bnode_remove(n);

		// refresh circle events
		push_circle(v, pa);
		push_circle(v, na);

		// start new segment
		vr_segment_t* s = new_segment(v, pa->r1, na->r1);
		s->s.a = &e->p->p;
		n->end = (vr_point_t**) &s->s.b;
	}
	else
	{
		vr_bnode_t* n = vr_binbeach_breakAt(&v->front, v->sweepline, e->r);

		if (n->left == NULL)
		{
			free(e);
			return 1;
		}

		// insert events
		push_circle(v, n->left);
		push_circle(v, n->right->right);

		// add segment
		vr_segment_t* s = new_segment(v, n->r1, e->r);
		n       ->end = (vr_point_t**) &s->s.a;
		n->right->end = (vr_point_t**) &s->s.b;
	}

	free(e);
	return 1;
}

static void finishSegments(vr_diagram_t* v, vr_bnode_t* n)
{
	if (n->left == NULL)
		return;

	vr_point_t* p = CALLOC(vr_point_t, 1);
	new_point(v, p);
	parabola_intersect(&p->p, &n->r1->p, &n->r2->p, v->sweepline);
	*n->end = p;

	finishSegments(v, n->left);
	finishSegments(v, n->right);
}
static char inRect(point_t* p)
{
	return
	0 < p->x && p->x < 20 &&
	0 < p->y && p->y < 20 &&
	1;
}
static void vr_diagram_restrictRegion(vr_diagram_t* v, vr_region_t* r)
{
	point_t corners[4] =
	{
		{ 0, 0},
		{ 0,20},
		{20,20},
		{20, 0},
	};
	segment_t border[4] =
	{
		{&corners[0], &corners[1]},
		{&corners[1], &corners[2]},
		{&corners[2], &corners[3]},
		{&corners[3], &corners[0]},
	};

	// find two jutting edges
	point_t* a = NULL;
	point_t* b = NULL;
	for (ssize_t j = 0; j < (ssize_t) r->n_edges; j++)
	{
		vr_segment_t* s = r->edges[j];
		char ak = inRect(s->s.a);
		char bk = inRect(s->s.b);

		if (!ak && !bk) // outside edge
		{
			r->n_edges--;
			memmove(r->edges+j, r->edges+j+1, sizeof(segment_t*)*(r->n_edges-j));
			j--;
		}
		else if (ak != bk) // jutting edge
		{
			point_t* p = !ak ? s->s.a : s->s.b;
			if (a == NULL) // first jutting edge
			{
				// TODO
				// BEGIN quickfix
				// this edge might share its end with the other
				// jutting edge; we need to split this end
				vr_point_t* np = CALLOC(vr_point_t, 1);
				new_point(v, np);
				np->p = *p;
				p = &np->p;
				if (!ak) s->s.a = p;
				else s->s.b = p;
				// END quickfix

				a = p;
			}
			else // second jutting edge
			{
				b = p;
			}

			// crop the edge
			for (size_t k = 0; k < 4 && !segment_intersect(p, &border[k], &s->s); k++);
		}
	}

	if (a == NULL || b == NULL)
		return;

	point_t p;
	// handle sides
	if (a->x == b->x || a->y == b->y)
	{
		vr_segment_t* s = new_segment(v, r, NULL);
		s->s.a = a;
		s->s.b = b;
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
	vr_point_t* np = CALLOC(vr_point_t, 1);
	np->p = p;
	vr_segment_t* s1 = new_segment(v, r, NULL);
	vr_segment_t* s2 = new_segment(v, r, NULL);
	s1->s.a = a;
	s1->s.b = &np->p;
	s2->s.a = &np->p;
	s2->s.b = b;
}
void vr_diagram_end(vr_diagram_t* v)
{
	while (vr_diagram_step(v));

	v->sweepline += 1000;
	finishSegments(v, v->front.root);

	for (size_t i = 0; i < v->n_regions; i++)
		vr_diagram_restrictRegion(v, v->regions[i]);
}
