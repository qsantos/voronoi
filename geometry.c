#include "geometry.h"

#include <math.h>

point_t point_minus(point_t a, point_t b)
{
	return (point_t){a.x-b.x,a.y-b.y};
}

float point_cross(point_t a, point_t b)
{
	return a.x*b.y - a.y*b.x;
}

/*
Parabolas are defined as being the set of points equidistant from a
vertex (focus) and a line (directrix). It is exactly what we need for
our beachline where the directrix is the sweepline and the focus is the
considered site.

Considering focus f and directrix of equation x=p, the parabola is
then defined as |x-p| = sqrt((x-f.x)^2+(y-f.y)^2) (using Euclidean
distance). By squaring, we get the condition:

p^2 - 2px = f.x^2 - 2x*f.x + (y-f.y)^2
or
2x(f.x-p) = f.x^2 - p^2 + (y.fy)^2

Thus:
{
2x(f1.x-p) = f1.x^2 - p^2 + (y-f1.y)^2
2x(f2.x-p) = f2.x^2 - p^2 + (y-f2.y)^2
}

The first line gets us:
2x = (f1.x^2 - p^2 + (y-f1.y)^2) / (f1.x-p)

Replacing 2x in the second one, we infer that:

(f1.x^2 - p^2 + (y-f1.y)^2) r = f2.x^2 - p^2 + (y-f2.y)^2

For convenience, let:
r = (f2.x-p)/(f1.x-p)
d1 = f1.x^2 - p^2 + f1.y^2
d2 = f2.x^2 - p^2 + f2.y^2

We now have an equation in y:
(y-f1.y)^2 r - (y-f2.y)^2 = f1.x^2-p^2 - (f2.x^2-p^2)*r
or
y^2 * (r-1) + y * 2(f2.y-f1.y*r) + (d1*r-d2) = 0

Hence, with:
a = r-1
b = 2(f2.y-f1.y*r)
c = d1*r+d2
delta = b^2 - 4ac

We have: y = (-b +- sqrt(delta)) / (2a)
*/
char intersection(point_t* dst, point_t* f1, point_t* f2, float p)
{
	point_t* f = f1;

	if (f1->x == f2->x)
	{
		dst->y = (f1->y+f2->y)/2;
	}
	else if (f1->x == p)
	{
		dst->y = f1->y;
		f = f2;
	}
	else
	{
		float r = (f2->x-p)/(f1->x-p);
		float d1 = f1->x*f1->x - p*p + f1->y*f1->y;
		float d2 = f2->x*f2->x - p*p + f2->y*f2->y;

		float a = r-1;
		float b = 2*(f2->y - f1->y*r);
		float c = d1*r - d2;
		float delta = b*b - 4*a*c;

		if (delta < 0)
			return 0;
		dst->y = (-b + sqrt(delta)) / (2*a);
	}

	float t = dst->y-f->y;
	dst->x = (f->x*f->x - p*p + t*t) / (2*(f->x-p));
	return 1;
}

/*
Consider circle of center (x,y) and radius r. We are given three points
p1, p2 and p3 and want to find back the center an the radius. We have:

(p1.x-x)^2 + (p1.y-y)^2 = r^2
(p2.x-x)^2 + (p2.y-y)^2 = r^2
(p3.x-x)^2 + (p3.y-y)^2 = r^2

According to http://paulbourke.net/geometry/circlesphere/, it solves to:

a = (p2.y-p1.y) / (p2.x-p1.x)
b = (p3.y-p1.y) / (p3.x-p1.x)

x = (ab(p1.y-p3.y) + b(p1.x+p2.x) - a(p2.x+p3.x)) / (2*(b-a))
y = -(x - (p1.x+p2.x)/2)/a + (p1.y+p2.y)/2

	float a = (p2->y-p1->y) / (p2->x-p1->x);
	float b = (p3->y-p2->y) / (p3->x-p2->x);
	if (a == b)
		return 0;

	float x = (a*b*(p1->y-p3->y) + b*(p1->x+p2->x) - a*(p2->x+p3->x)) / (2*(b-a));
	float y = -(x - (p1->x+p2->x)/2)/a + (p1->y+p2->y)/2;

	float dx = p1->x - x;
	float dy = p1->y - y;
	*r = sqrt(dx*dx + dy*dy);

	c->x = x;
	c->y = y;

	return 1;
not working
*/

// from http://www.cs.hmc.edu/~mbrubeck/voronoi.html
char circle_from3(point_t* c, float* r, point_t* p1, point_t* p2, point_t* p3)
{
	// Check that bc is a "right turn" from ap2->
	if ((p2->x-p1->x)*(p3->y-p1->y) - (p3->x-p1->x)*(p2->y-p1->y) > 0)
		return 0;

	// Algorithm from O'Rourke 2ed p. 189.
	double A = p2->x - p1->x,  B = p2->y - p1->y,
	       C = p3->x - p1->x,  D = p3->y - p1->y,
	       E = A*(p1->x+p2->x) + B*(p1->y+p2->y),
	       F = C*(p1->x+p3->x) + D*(p1->y+p3->y),
	       G = 2*(A*(p3->y-p2->y) - B*(p3->x-p2->x));

	if (G == 0) return 0;  // Points are co-linear.

	// Point o is the center of the circle.
	c->x = (D*E-B*F)/G;
	c->y = (A*F-C*E)/G;

	// o.x plus radius equals max x coordinate.
	*r = sqrt( pow(p1->x-c->x, 2) + pow(p1->y-c->y, 2) );
	return 1;
}

// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282
char segment_intersect(point_t* dst, segment_t* a, segment_t* b)
{
	point_t p = a->a;
	point_t r = point_minus(a->b,p);
	point_t q = b->a;
	point_t s = point_minus(b->b,q);

	float y = point_cross(r, s);
	if (y != 0)
	{
		point_t m = point_minus(q, p);
		float t = point_cross(m, s) / y;
		float u = point_cross(m, r) / y;
		if (0 <= t && t <= 1 && 0 <= u && u <= 1)
		{
			dst->x = p.x + t*r.x;
			dst->y = p.y + t*r.y;
			return 1;
		}
	}
	return 0;
}
