#include "lloyd.h"

#include <stdlib.h>
#include <math.h>

#define M_PI 3.14159265358979323846

static float heading(point_t p)
{
	float dir;
	if (p.x > 0)
	{
		dir = atan(p.y / p.x);
		if (p.y < 0)
			dir += 2 * M_PI;
	}
	else if (p.x < 0)
	{
		dir = atan(p.y / p.x) + M_PI;
	}
	else // p.x == 0
	{
		dir = M_PI / 2;
		if (p.y < 0)
			dir += M_PI;
	}
	return dir;
}
static int poly_vert(const void* a, const void* b, void* arg)
{
	const point_t* pa = (const point_t*) a;
	const point_t* pb = (const point_t*) b;
	point_t* c = (point_t*) arg;
	float da = heading((point_t){pa->x-c->x, pa->y-c->y});
	float db = heading((point_t){pb->x-c->x, pb->y-c->y});
	return da < db ? -1 : da > db ? +1 : 0;
}
void lloyd_relaxation(voronoi_t* v)
{
	voronoi_end(v);

	point_t npoints[v->n_regions];
	size_t k = 0;
	for (size_t i = 0; i < v->n_regions; i++)
	{
		region_t* r = v->regions[i];

		point_t v2[2*r->n_edges];
		for (size_t j = 0; j < r->n_edges; j++)
		{
			size_t id = r->edges[j];
			segment_t* s = voronoi_id2segment(v, id);
			v2[2*j]   = s->a;
			v2[2*j+1] = s->b;
		}

		point_t centroid;
		for (size_t j = 0; j < 2*r->n_edges; j++)
		{
			centroid.x += v2[j].x;
			centroid.y += v2[j].y;
		}
		centroid.x /= 2*r->n_edges;
		centroid.y /= 2*r->n_edges;

		qsort_r(v2, 2*r->n_edges, sizeof(point_t), poly_vert, &centroid);

		point_t v[r->n_edges];
		for (size_t j = 0; j < r->n_edges; j++)
			v[j] = v2[2*j];

		float x = 0;
		float y = 0;
		float A = 0;
		for (size_t i=0, j=r->n_edges-1; i < r->n_edges; j=i++)
		{
			point_t p = v[i];
			point_t q = v[j];
			float f = p.x*q.y - q.x*p.y;
			x += (p.x+q.x)*f;
			y += (p.y+q.y)*f;
			A += f;
		}
		A /= 2;
		x /= 6*A;
		y /= 6*A;

		if (fabs(A) > 100)
			continue;

		if (0 <= x && x <= 20 && 0 <= y && y <= 20)
		{
			npoints[k].x = x;
			npoints[k].y = y;
			k++;
		}
	}
	voronoi_exit(v);
	voronoi_init(v);
	voronoi_points(v, k, npoints);
}
