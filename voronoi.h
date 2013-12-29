#ifndef VORONOI_H
#define VORONOI_H

#include "heap.h"
#include "geometry.h"

typedef struct beach   beach_t;
typedef struct event   event_t;
typedef struct voronoi voronoi_t;

struct beach
{
	point_t p;

	segment_t* s1;
	segment_t* s2;

	beach_t* next;
	beach_t* prev;

	event_t* e;
};

struct event
{
	// 0 if new vertex, 1 if circle
	char is_circle;
	char active;

	point_t p; // vertex
	beach_t* l; // circle info
};

struct voronoi
{
	heap_t        events;
	beach_t* front;
	float         sweepline;

	size_t      n_segments;
	size_t      a_segments;
	segment_t** segments;
};

void voronoi_init(voronoi_t* v);
void voronoi_exit(voronoi_t* v);

void voronoi_point (voronoi_t* v, point_t p);
void voronoi_points(voronoi_t* v, size_t n, point_t* p);

char voronoi_step(voronoi_t* v);

#endif
