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

#ifndef VORONOI_H
#define VORONOI_H

typedef struct vr_point   vr_point_t;
typedef struct vr_segment vr_segment_t;
typedef struct vr_region  vr_region_t;
typedef struct vr_event   vr_event_t;
typedef struct vr_diagram vr_diagram_t;

#include "heap.h"
#include "geometry.h"
#include "binbeach.h"

struct vr_point
{
	point_t p;
};

struct vr_segment
{
	segment_t s;

	vr_region_t* ra;
	vr_region_t* rb;
};

struct vr_region
{
	// site
	point_t p;

	size_t         n_edges;
	vr_segment_t** edges;
};

struct vr_event
{
	// 0 if new vertex, 1 if circle
	char is_circle;
	char active;

	vr_region_t* r;

	// circle info
	vr_point_t* p;
	vr_bnode_t* n;
};

struct vr_diagram
{
	size_t       n_points;
	size_t       a_points;
	vr_point_t** points;

	size_t         n_segments;
	size_t         a_segments;
	vr_segment_t** segments;

	size_t        n_regions;
	size_t        a_regions;
	vr_region_t** regions;

	heap_t        events;
	vr_binbeach_t front;
	double        sweepline;
};

void vr_diagram_init(vr_diagram_t* v);
void vr_diagram_exit(vr_diagram_t* v);

void vr_diagram_point (vr_diagram_t* v, point_t p);
void vr_diagram_points(vr_diagram_t* v, size_t n, point_t* p);

char vr_diagram_step(vr_diagram_t* v);
void vr_diagram_end (vr_diagram_t* v);

#endif
