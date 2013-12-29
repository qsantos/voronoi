#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <GL/freeglut.h>

#include "utils.h"
#include "voronoi.h"

voronoi_t v;

static void glInit()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPointSize(3);
	glOrtho(0, 20, 20, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
}

static void draw_parabola(point_t* f, float p, float y1, float y2)
{
	if (f->x == p)
	{
		glVertex2f(f->x, f->y);
		return;
	}

	y1 = fmax(y1, 0);
	y2 = fmin(y2, 20);

	for (float y = y1; y < y2; y+=0.1)
	{
		float t = y-f->y;
		float x = (f->x*f->x - p*p + t*t) / (2*(f->x-p));
		glVertex2f(x, y);
	}

	float t = y2-f->y;
	float x = (f->x*f->x - p*p + t*t) / (2*(f->x-p));
	glVertex2f(x, y2);
}

static void cb_display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	// sites
	glColor4ub(255, 0, 255, 255);
	glBegin(GL_POINTS);
	for (size_t i = 0; i < v.n_regions; i++)
	{
		point_t* p = &v.regions[i]->p;
		glVertex2f(p->x, p->y);
	}
	glEnd();

	// arcs
	glColor4ub(255, 0, 0, 255);
	glBegin(GL_LINE_STRIP);
	for (arc_t* l = v.front; l; l = l->next)
	{
		float y1 =  0;
		float y2 = 20;

		if (l->prev != NULL)
		{
			point_t p;
			intersection(&p, &l->prev->r->p, &l->r->p, v.sweepline);
			y1 = p.y;

			segment_t* s = &v.segments[l->end/2];
			if (l->end%2==0) s->a = p;
			else             s->b = p;
		}
		if (l->next != NULL)
		{
			point_t p;
			intersection(&p, &l->r->p, &l->next->r->p, v.sweepline);
			y2 = p.y;
		}

		draw_parabola(&l->r->p, v.sweepline, y1, y2);
	}
	glEnd();

	// segments
	glColor4ub(255, 255, 255, 255);
	glBegin(GL_LINES);
	for (size_t i = 0; i < v.n_segments; i++)
	{
		segment_t* s = &v.segments[i];
		glVertex2f(s->a.x, s->a.y);
		glVertex2f(s->b.x, s->b.y);
	}
	glEnd();

	// sweepline
	glColor4ub(0, 255, 0, 255);
	glBegin(GL_LINES);
	glVertex2f(v.sweepline, 0);
	glVertex2f(v.sweepline, 20);
	glEnd();

	glutSwapBuffers();
}

static void cb_keyboard(unsigned char c, int x, int y)
{
	(void) x;
	(void) y;

	if (c == ' ')
	{
		v.sweepline += 0.05;
		if (v.sweepline >= v.events.tree[0].idx)
			voronoi_step(&v);
	}
	else if (c == '\r')
		voronoi_end(&v);
	else if (c == 'a')
		for (size_t i = 0; i < 1000; i++)
			voronoi_step(&v);
	else if (c == 'z')
		for (size_t i = 0; i < 100; i++)
			voronoi_step(&v);
	else if (c == 'e')
		for (size_t i = 0; i < 10; i++)
			voronoi_step(&v);
	else
		voronoi_step(&v);

	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	char glEnabled = 0;
	size_t n_points = 10000;

	int curarg = 1;
	if (curarg < argc)
		n_points = atoi(argv[curarg++]);
	if (curarg < argc)
	{
		glEnabled = 1;
		curarg++;
	}

	voronoi_init(&v);

	srand(42);
	for (size_t i = 0; i < n_points; i++)
	{
		float x = ( (float) rand() / INT_MAX ) * 20;
		float y = ( (float) rand() / INT_MAX ) * 20;
		voronoi_point(&v, (point_t){x,y});
	}

	if (glEnabled)
	{
		glutInit(&argc, argv);
		glutInitWindowSize(800, 600);
		glutCreateWindow("Voronoi");
		glutSetCursor(GLUT_CURSOR_NONE);
		glutDisplayFunc (&cb_display);
		glutKeyboardFunc(&cb_keyboard);
		glInit();
		glutMainLoop();
	}
	else
	{
		voronoi_end(&v);
	}

	voronoi_exit(&v);
	return 0;
}
