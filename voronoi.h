#ifndef VORONOI_H
#define VORONOI_H

#include "heap.h"
#include "geometry.h"

typedef struct region  region_t;
typedef struct arc     arc_t;
typedef struct event   event_t;
typedef struct voronoi voronoi_t;

struct region
{
	// site
	point_t p;

	size_t     n_edges;
	segment_t* edges;
};

struct arc
{
	region_t* r;

	size_t end;

	arc_t* next;
	arc_t* prev;

	event_t* e;
};

struct event
{
	// 0 if new vertex, 1 if circle
	char is_circle;
	char active;

	region_t* r;

	// circle info
	point_t p;
	arc_t* l;
};

struct voronoi
{
	size_t     n_regions;
	size_t     a_regions;
	region_t** regions;

	heap_t events;
	arc_t* front;
	float  sweepline;

	size_t     n_segments;
	size_t     a_segments;
	segment_t* segments;
};

void voronoi_init(voronoi_t* v);
void voronoi_exit(voronoi_t* v);

void voronoi_point (voronoi_t* v, point_t p);
void voronoi_points(voronoi_t* v, size_t n, point_t* p);

char voronoi_step(voronoi_t* v);
void voronoi_end (voronoi_t* v);

#endif
