#include "voronoi.h"

#include "utils.h"

void voronoi_init(voronoi_t* v)
{
	heap_init(&v->events);
	v->front = NULL;
	v->sweepline = 0;
}

void voronoi_exit(voronoi_t* v)
{
	point_list_t* l = v->front;
	while (l)
	{
		point_list_t* next = l->next;
		free(l);
		l = next;
	}
	heap_exit(&v->events);
}

void voronoi_point(voronoi_t* v, point_t p)
{
	event_t* e = CALLOC(event_t, 1);
	*e = (event_t){0, p, NULL};
	heap_insert(&v->events, p.x, e);
}

void voronoi_points(voronoi_t* v, size_t n, point_t* p)
{
	for (; n; p++, n--)
		voronoi_point(v, *p);
}

static void push_circle(voronoi_t* v, point_list_t* l)
{
	if (l == NULL || l->prev == NULL || l->next == NULL)
		return;

	event_t* e = CALLOC(event_t, 1);
	float r;
	if (!circle_from3(&e->p, &r, &l->prev->p, &l->p, &l->next->p))
		return;

	e->p.x += r;
	if (e->p.x < v->sweepline)
		return;

	e->is_circle = 1;
	e->l = l;

	heap_insert(&v->events, e->p.x, e);
}
void voronoi_step(voronoi_t* v)
{
	event_t* e = heap_remove(&v->events);
	if (e == NULL)
		return;
	v->sweepline = e->p.x;

	if (e->is_circle)
	{
		point_list_t* l = e->l;
		if (l->prev)
			l->prev->next = l->next;
		if (l->next)
			l->next->prev = l->prev;
		push_circle(v, l->prev);
		push_circle(v, l->next);
		return;
	}

	if (v->front == NULL)
	{
		point_list_t* a = CREALLOC(v->front, point_list_t, 1);
		a->p = e->p;
		a->next = NULL;
		a->prev = NULL;
		v->front = a;
		return;
	}

	point_list_t* l = v->front;
	for (; l; l = l->next)
	{
		// intersection with i-th arc
		point_t p;
		intersection(&p, &e->p, &l->p, v->sweepline);

		// check for previous breakpoint
		if (l->prev != NULL)
		{
			point_t q;
			intersection(&q, &l->prev->p, &l->p, v->sweepline);
			if (p.y < q.y)
				continue;
		}

		// check for next breakpoint
		if (l->next != NULL)
		{
			point_t q;
			intersection(&q, &l->p, &l->next->p, v->sweepline);
			if (p.y > q.y)
				continue;
		}
		break;
	}

	point_list_t* a = CALLOC(point_list_t, 1);
	point_list_t* b = CALLOC(point_list_t, 1);

	a->next = b;
	a->prev = l;
	a->p = e->p;

	b->prev = a;
	b->next = l->next;
	b->p = l->p;

	if (l->next)
		l->next->prev = b;

	l->next = a;

	push_circle(v, a->prev);
	push_circle(v, a->next);
}
