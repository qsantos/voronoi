#ifndef VORONOI_H
#define VORONOI_H

#include "heap.h"
#include "geometry.h"

typedef struct point_list point_list_t;
typedef struct event      event_t;
typedef struct voronoi    voronoi_t;

struct point_list
{
	point_t p;

	point_list_t* next;
	point_list_t* prev;

	event_t* e;
};

struct event
{
	// 0 if new vertex, 1 if circle
	char is_circle;
	char active;

	point_t p; // vertex
	point_list_t* l; // circle info
};

struct voronoi
{
	heap_t        events;
	point_list_t* front;
	float         sweepline;
};

void voronoi_init(voronoi_t* v);
void voronoi_exit(voronoi_t* v);

void voronoi_point (voronoi_t* v, point_t p);
void voronoi_points(voronoi_t* v, size_t n, point_t* p);

void voronoi_step(voronoi_t* v);

#endif
