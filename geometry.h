#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct point   point_t;
typedef struct segment segment_t;

struct point
{
	float x;
	float y;
};

struct segment
{
	point_t a;
	point_t b;
};

point_t point_minus(point_t a, point_t b);
float   point_cross(point_t a, point_t b);

// compute the parabola_intersect of two parabola of
// focuses f1 and f2 and common directrix x=p
char parabola_intersect(point_t* dst, point_t* f1, point_t* f2, float p);

// compute the center c and radius r of a circle
// passing through three given point p1, p2 and p3
char circle_from3(point_t* c, float* r, point_t* p1, point_t* p2, point_t* p3);

// compute the parabola_intersect between two segments
char segment_intersect(point_t* dst, segment_t* a, segment_t* b);

#endif
