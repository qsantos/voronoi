#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "utils.h"
#include "voronoi.h"

voronoi_t v;
point_t* points;
size_t n_points;

#include "gl.c"

int main(int argc, char** argv)
{
	char glEnabled = 0;
	n_points = 10000;

	int curarg = 1;
	if (curarg < argc)
		n_points = atoi(argv[curarg++]);
	if (curarg < argc)
	{
		glEnabled = 1;
		curarg++;
	}

	points = CALLOC(point_t, n_points);

	srand(42);
	for (size_t i = 0; i < n_points; i++)
	{
		points[i].x = ( (float) rand() / INT_MAX ) * 20;
		points[i].y = ( (float) rand() / INT_MAX ) * 20;
	}

	voronoi_init(&v);
	voronoi_points(&v, n_points, points);

	if (glEnabled)
	{
		glutInit(&argc, argv);
		glutInitWindowSize(winWidth, winHeight);
		glutCreateWindow("Voronoi");
		glutSetCursor(GLUT_CURSOR_NONE);
		glutDisplayFunc (&cb_display);
		glutKeyboardFunc(&cb_keyboard);
		glInit();
		glutMainLoop();
	}
	else
	{
		voronoi_do(&v);
	}

	voronoi_exit(&v);
	free(points);
	return 0;
}
