#include <GL/freeglut.h>

#include "voronoi.h"

float winWidth  = 800;
float winHeight = 600;

static void glInit()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPointSize(3);

	// two dimensionnal mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 20, 20, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);

	// enables transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void draw_parabola(point_t* f, float p, float y1, float y2)
{
	if (f->x == p)
	{
		glVertex2f(f->x, f->y);
		return;
	}

	if (y2 < 0 || y1 > 20)
		return;

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

	glColor4ub(255, 0, 255, 255);
	glBegin(GL_POINTS);
	for (size_t i = 0; i < n_points; i++)
		glVertex2f(points[i].x, points[i].y);
	glEnd();

	glColor4ub(255, 0, 0, 255);
	glBegin(GL_LINE_STRIP);
	for (beach_t* l = v.front; l; l = l->next)
	{
		float y1 =  0;
		float y2 = 20;

		if (l->prev != NULL)
		{
			point_t p;
			intersection(&p, &l->prev->p, &l->p, v.sweepline);
			l->s1->b = p;
			y1 = p.y;
		}
		if (l->next != NULL)
		{
			point_t p;
			intersection(&p, &l->p, &l->next->p, v.sweepline);
			l->s2->b = p;
			y2 = p.y;
		}

		draw_parabola(&l->p, v.sweepline, y1, y2);
	}
	glEnd();

	glColor4ub(255, 255, 255, 255);
	glBegin(GL_POINTS);
	for (beach_t* l = v.front; l; l = l->next)
	{
		if (l->prev == NULL)
			continue;

		point_t p;
		intersection(&p, &l->prev->p, &l->p, v.sweepline);
		glVertex2f(p.x, p.y);
	}
	glEnd();

	glColor4ub(255, 255, 255, 255);
	glBegin(GL_LINES);
	for (size_t i = 0; i < v.n_segments; i++)
	{
		segment_t* s = v.segments[i];
		glVertex2f(s->a.x, s->a.y);
		glVertex2f(s->b.x, s->b.y);
	}
	glEnd();

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

	glutPostRedisplay();

	if (c == ' ')
	{
		v.sweepline += 0.05;
		if (v.sweepline >= v.events.tree[0].idx)
			voronoi_step(&v);
	}
	else if (c == '\r')
	{
		while (voronoi_step(&v));
	}
	else if (c == '\t')
	{
		for (size_t i = 0; i < 100; i++)
			voronoi_step(&v);
	}
	else
		voronoi_step(&v);
}
