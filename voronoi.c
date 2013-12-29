#include "voronoi.h"

#include "utils.h"

void voronoi_init(voronoi_t* v)
{
	heap_init(&v->events);
	v->front      = NULL;
	v->sweepline  = 0;
	v->n_segments = 0;
	v->a_segments = 0;
	v->segments   = NULL;
}

void voronoi_exit(voronoi_t* v)
{
	free(v->segments);
	beach_t* l = v->front;
	while (l)
	{
		beach_t* next = l->next;
		free(l);
		l = next;
	}
	heap_exit(&v->events);
}

void voronoi_point(voronoi_t* v, point_t p)
{
	event_t* e = CALLOC(event_t, 1);
	*e = (event_t){0, 1, p, NULL};
	heap_insert(&v->events, p.x, e);
}

void voronoi_points(voronoi_t* v, size_t n, point_t* p)
{
	for (; n; p++, n--)
		voronoi_point(v, *p);
}

static void push_circle(voronoi_t* v, beach_t* l)
{
	if (l == NULL)
		return;

	if (l->e != NULL && l->e->p.x != v->sweepline)
		l->e->active = 0;
	l->e = NULL;

	if (l == NULL || l->prev == NULL || l->next == NULL)
		return;

	event_t* e = CALLOC(event_t, 1);
	e->active = 1;
	float r;
	if (!circle_from3(&e->p, &r, &l->prev->p, &l->p, &l->next->p))
		return;

	e->is_circle = 1;
	e->l = l;
	heap_insert(&v->events, e->p.x + r, e);
	l->e = e;
}
static segment_t* new_segment(voronoi_t* v, point_t p)
{
	if (v->n_segments == v->a_segments)
	{
		v->a_segments = v->a_segments == 0 ? 1 : 2*v->a_segments;
		v->segments = CREALLOC(v->segments, segment_t*, v->a_segments);
	}
	segment_t* s = CALLOC(segment_t, 1);
	s->a = p;
	s->b = p;
	v->segments[v->n_segments++] = s;
	return s;
}
char voronoi_step(voronoi_t* v)
{
	float idx = 0;
	if (v->events.size != 0)
		idx = v->events.tree[0].idx;

	event_t* e = heap_remove(&v->events);
	if (e == NULL)
		return 0;

	if (!e->active)
		return 1;

	v->sweepline = idx;

	if (e->is_circle)
	{
		beach_t* l = e->l;

		// finish segments
		if (l->s1) l->s1->b = e->p;
		if (l->s2) l->s2->b = e->p;

		// merge points
		if (l->prev != NULL)
			l->prev->next = l->next;
		if (l->next != NULL)
			l->next->prev = l->prev;

		// refresh circle events
		push_circle(v, l->prev);
		push_circle(v, l->next);

		segment_t* s = new_segment(v, e->p);
		if (l->prev != NULL)
			l->prev->s2 = s;
		if (l->next != NULL)
			l->next->s1 = s;
		return 1;
	}

	if (v->front == NULL)
	{
		beach_t* a = CALLOC(beach_t, 1);
		a->p = e->p;
		a->next = NULL;
		a->prev = NULL;
		a->s1 = NULL;
		a->s2 = NULL;
		a->e = NULL;
		v->front = a;
		return 1;
	}

	point_t p;
	beach_t* l = v->front;
	for (; l; l = l->next)
	{
		// intersection with i-th arc
		intersection(&p, &e->p, &l->p, v->sweepline);

		// check for previous breakpoint
		if (l->prev != NULL)
		{
			point_t q;
			if (!intersection(&q, &l->prev->p, &l->p, v->sweepline))
				continue;
			if (p.y < q.y)
				continue;
		}

		// check for next breakpoint
		if (l->next != NULL)
		{
			point_t q;
			if (!intersection(&q, &l->p, &l->next->p, v->sweepline))
				continue;
			if (p.y > q.y)
				continue;
		}
		break;
	}

	if (l == NULL)
		return 1;

	// insert arc
	beach_t* a = CALLOC(beach_t, 1);
	beach_t* b = CALLOC(beach_t, 1);

	a->e = NULL;
	b->e = NULL;

	a->next = b;
	a->prev = l;
	a->p = e->p;

	b->prev = a;
	b->next = l->next;
	b->p = l->p;

	b->s2 = l->s2;

	if (l->next)
		l->next->prev = b;

	l->next = a;

	// insert events
	push_circle(v, a->prev);
	push_circle(v, a->next);

	// add segment
	segment_t* s1 = new_segment(v, p);
	segment_t* s2 = new_segment(v, p);
	l->s2 = s1;
	a->s1 = s1;
	a->s2 = s2;
	b->s1 = s2;

	return 1;
}
