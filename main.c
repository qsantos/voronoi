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

#include <limits.h>
#include <math.h>
#include <GL/freeglut.h>

#include "utils.h"
#include "voronoi.h"
#include "lloyd.h"

int win_id;
vr_diagram_t v;

#define VR_WIDTH  800
#define VR_HEIGHT 600

static void glInit()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPointSize(3);
	glOrtho(-10, VR_WIDTH+10, VR_HEIGHT+10, -10, 0, 1);
	glDisable(GL_DEPTH_TEST);
}

static void draw_parabola(point_t* f, double p, double y1, double y2)
{
	if (f->x == p)
	{
		glVertex2f(f->x, f->y);
		return;
	}

	y1 = fmax(y1, 0);
	y2 = fmin(y2, VR_HEIGHT);

	for (double y = y1; y < y2; y+=0.1)
	{
		double t = y-f->y;
		double x = (f->x*f->x - p*p + t*t) / (2*(f->x-p));
		glVertex2f(x, y);
	}

	double t = y2-f->y;
	double x = (f->x*f->x - p*p + t*t) / (2*(f->x-p));
	glVertex2f(x, y2);
}

static void draw_beach(vr_bnode_t* n, double sweep, double miny, double maxy)
{
	if (n == NULL)
		return;

	if (n->left != NULL)
	{
		point_t p;
		parabola_intersect(&p, &n->r1->p, &n->r2->p, v.sweepline);

		draw_beach(n->left,  sweep, miny, p.y);
		draw_beach(n->right, sweep, p.y, maxy);
	}
	else
		draw_parabola(&n->r1->p, v.sweepline, miny, maxy);
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

	// beachline
	glColor4ub(255, 0, 0, 255);
	glBegin(GL_LINE_STRIP);
	draw_beach(v.front.root, v.sweepline, 0, VR_HEIGHT);
	glEnd();

	// segments
	glColor4ub(255, 255, 255, 255);
	glBegin(GL_LINES);
	for (size_t i = 0; i < v.n_regions; i++)
	{
		vr_region_t* r = v.regions[i];
		for (size_t j = 0; j < r->n_edges; j++)
		{
			segment_t* s = &r->edges[j]->s;
			if (s->a == NULL || s->b == NULL)
				continue;
			glVertex2f(s->a->x, s->a->y);
			glVertex2f(s->b->x, s->b->y);
		}
	}
	glEnd();

	// sweepline
	glColor4ub(0, 255, 0, 255);
	glBegin(GL_LINES);
	glVertex2f(v.sweepline, 0);
	glVertex2f(v.sweepline, VR_HEIGHT);
	glEnd();

	glutSwapBuffers();
}

static void cb_keyboard(unsigned char c, int x, int y)
{
	(void) x;
	(void) y;

	if (c == 27)
	{
		glutDestroyWindow(win_id);
		vr_diagram_exit(&v);
		exit(0);
	}
	else if (c == ' ')
	{
		v.sweepline += 0.05;
		if (v.sweepline >= v.events.tree[0].idx)
			vr_diagram_step(&v);
	}
	else if (c == 'l')
		vr_lloyd_relaxation(&v);
	else if (c == '\r')
		vr_diagram_end(&v);
	else if (c == 'a')
		for (size_t i = 0; i < 1000; i++)
			vr_diagram_step(&v);
	else if (c == 'z')
		for (size_t i = 0; i < 100; i++)
			vr_diagram_step(&v);
	else if (c == 'e')
		for (size_t i = 0; i < 10; i++)
			vr_diagram_step(&v);
	else
		vr_diagram_step(&v);

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

	vr_diagram_init(&v, VR_WIDTH, VR_HEIGHT);

	srand(42);
	for (size_t i = 0; i < n_points; i++)
	{
		double x = ( (double) rand() / INT_MAX ) * VR_WIDTH;
		double y = ( (double) rand() / INT_MAX ) * VR_HEIGHT;
		vr_diagram_point(&v, (point_t){x,y});
	}

	if (glEnabled)
	{
		glutInit(&argc, argv);
		glutInitWindowSize(VR_WIDTH+20, VR_HEIGHT+20);
		win_id = glutCreateWindow("Voronoi");
		glutSetCursor(GLUT_CURSOR_NONE);
		glutDisplayFunc (&cb_display);
		glutKeyboardFunc(&cb_keyboard);
		glInit();
		glutMainLoop();
	}
	else
	{
		vr_diagram_end(&v);
		vr_diagram_exit(&v);
		return 0;
	}
}
