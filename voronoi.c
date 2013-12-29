#include "voronoi.h"

#include "utils.h"

void voronoi_init(voronoi_t* v)
{
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
	for (size_t i = 0; i < v->n_segments; i++)
		free(v->segments[i]);
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
	float r;
	if (!circle_from3(&e->p, &r, &l->prev->r->p, &l->r->p, &l->next->r->p))
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
	{
		free(e);
		return 1;
	}

	v->sweepline = idx;

	if (e->is_circle)
	{
		arc_t* l = e->l;

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
		{
			l->prev->r->n_edges++;
			l->prev->s2 = s;
		}
		if (l->next != NULL)
		{
			l->next->r->n_edges++;
			l->next->s1 = s;
		}

		free(l);
		free(e);
		return 1;
	}

	if (v->front == NULL)
	{
		arc_t* a = CALLOC(arc_t, 1);
		a->r = e->r;
		a->next = NULL;
		a->prev = NULL;
		a->s1 = NULL;
		a->s2 = NULL;
		a->e = NULL;
		v->front = a;
		free(e);
		return 1;
	}

	point_t p;
	arc_t* l = v->front;
	for (; l; l = l->next)
	{
		// intersection with i-th arc
		intersection(&p, &e->r->p, &l->r->p, v->sweepline);

		// check for previous breakpoint
		if (l->prev != NULL)
		{
			point_t q;
			if (!intersection(&q, &l->prev->r->p, &l->r->p, v->sweepline))
				continue;
			if (p.y < q.y)
				continue;
		}

		// check for next breakpoint
		if (l->next != NULL)
		{
			point_t q;
			if (!intersection(&q, &l->r->p, &l->next->r->p, v->sweepline))
				continue;
			if (p.y > q.y)
				continue;
		}
		break;
	}

	if (l == NULL)
	{
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

	a->r->n_edges++;

	free(e);
	return 1;
}
void voronoi_end(voronoi_t* v)
{
	while (voronoi_step(v));

	v->sweepline += 1000;
	for (arc_t* l = v->front; l->next; l = l->next)
	{
		point_t p;
		intersection(&p, &l->r->p, &l->next->r->p, v->sweepline);
		if (l->s1)
			l->s1->b = p;
	}
}
