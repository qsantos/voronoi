#ifndef VORONOI_H
#define VORONOI_H

typedef struct region  region_t;
typedef struct event   event_t;
typedef struct voronoi voronoi_t;

#include "heap.h"
#include "geometry.h"
#include "binbeach.h"

struct region
{
	// site
	point_t p;

	size_t      n_edges;
	size_t*     edge_ids;
	segment_t** edges;
};

struct event
{
	// 0 if new vertex, 1 if circle
	char is_circle;
	char active;

	region_t* r;

	// circle info
	point_t  p;
	bnode_t* n;
};

struct voronoi
{
	char done;

	size_t     n_regions;
	size_t     a_regions;
	region_t** regions;

	heap_t     events;
	binbeach_t front;
	double     sweepline;

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

// fill in quick access pointers in structures:
//   * region_t.edges
void voronoi_ptrs(voronoi_t* v);

point_t*   voronoi_id2point  (voronoi_t* v, size_t id);
segment_t* voronoi_id2segment(voronoi_t* v, size_t id);

#endif
