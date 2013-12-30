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

	v->front      = NULL;
	v->sweepline  = 0;
	v->n_segments = 0;
	v->a_segments = 0;
	v->segments   = NULL;
}

void voronoi_exit(voronoi_t* v)
{
	free(v->segments);

	arc_t* l = v->front;
	while (l)
	{
		arc_t* next = l->next;
		free(l);
		l = next;
	}

	event_t* e;
	while ((e = heap_remove(&v->events)) != NULL)
		free(e);
	heap_exit(&v->events);

	for (size_t i = 0; i < v->n_regions; i++)
		free(v->regions[i]);
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
	*r = (region_t){p, 0, NULL};
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

static void push_circle(voronoi_t* v, arc_t* l)
{
	if (l == NULL)
		return;

	if (l->e != NULL && l->e->p.x != v->sweepline)
		l->e->active = 0;
	l->e = NULL;

	if (l->prev == NULL || l->next == NULL)
		return;

	event_t* e = CALLOC(event_t, 1);
	e->active = 1;
	e->r = l->r;
	double r;
	if (!circle_from3(&e->p, &r, &l->prev->r->p, &l->r->p, &l->next->r->p))
	{
		free(e);
		return;
	}

	e->is_circle = 1;
	e->l = l;
	heap_insert(&v->events, e->p.x + r, e);
	l->e = e;
}
static void push_segment(region_t* a, size_t e)
{
	a->edges = CREALLOC(a->edges, size_t, a->n_edges+1);
	a->edges[a->n_edges++] = e;
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
		arc_t* l = e->l;

		// finish segments
		*voronoi_id2point(v, l->end) = e->p;
		*voronoi_id2point(v, l->next->end) = e->p;

		// merge points
		l->prev->next = l->next;
		l->next->prev = l->prev;

		// refresh circle events
		push_circle(v, l->prev);
		push_circle(v, l->next);

		// start new segment
		size_t s = new_segment(v, l->prev->r, l->next->r);
		*voronoi_id2point(v, 2*s) = e->p;
		l->next->end = 2*s+1;

//		free(l);
		free(e);
		return 1;
	}

	if (v->front == NULL)
	{
		arc_t* a = CALLOC(arc_t, 1);
		a->r = e->r;
		a->next = NULL;
		a->prev = NULL;
		a->end = 0;
		a->e = NULL;
		v->front = a;
		free(e);
		return 1;
	}

	arc_t* l = v->front;
	for (; l; l = l->next)
	{
		// parabola_intersect with i-th arc
		point_t p;
		parabola_intersect(&p, &e->r->p, &l->r->p, v->sweepline);

		// check for previous and next breakpoints
		point_t q;
		if (l->prev != NULL && (!parabola_intersect(&q, &l->prev->r->p, &l->r->p, v->sweepline) || p.y < q.y))
			continue;
		if (l->next != NULL && (!parabola_intersect(&q, &l->r->p, &l->next->r->p, v->sweepline) || p.y > q.y))
			continue;

		break;
	}

	if (l == NULL)
	{
		// TODO
		free(e);
		return 1;
	}

	// insert arc
	arc_t* a = CALLOC(arc_t, 1);
	arc_t* b = CALLOC(arc_t, 1);

	a->e = NULL;
	b->e = NULL;

	a->next = b;
	a->prev = l;
	a->r = e->r;

	b->prev = a;
	b->next = l->next;
	b->r = l->r;

	if (l->next)
		l->next->prev = b;

	l->next = a;

	// insert events
	push_circle(v, a->prev);
	push_circle(v, a->next);

	// add segment
	size_t s = new_segment(v, l->r, e->r);
	a->end = 2*s;
	b->end = 2*s+1;

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
static void voronoi_finishSegments(voronoi_t* v)
{
	v->sweepline += 1000;
	if (v->front == NULL)
		return;
	for (arc_t* l = v->front->next; l; l = l->next)
	{
		point_t p;
		parabola_intersect(&p, &l->prev->r->p, &l->r->p, v->sweepline);
		*voronoi_id2point(v, l->end) = p;
	}
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
		segment_t* s = voronoi_id2segment(v, r->edges[j]);
		char ak = inRect(&s->a);
		char bk = inRect(&s->b);

		if (!ak && !bk)
		{
			r->n_edges--;
			memmove(r->edges+j, r->edges+j+1, sizeof(size_t)*(r->n_edges-j));
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

	voronoi_finishSegments(v);

	for (size_t i = 0; i < v->n_regions; i++)
		voronoi_restrictRegion(v, v->regions[i]);

	v->done = 1;
}

point_t* voronoi_id2point(voronoi_t* v, size_t id)
{
	segment_t* s = &v->segments[id/2];
	return id % 2 == 0 ? &s->a : &s->b;
}
segment_t* voronoi_id2segment(voronoi_t* v, size_t id)
{
	return &v->segments[id];
}
