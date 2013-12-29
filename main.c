#include <string.h>
#include <limits.h>

#include "voronoi.h"

voronoi_t v;
point_t points[20];
size_t n_points;

#include "gl.c"

int main(int argc, char** argv)
{
	n_points = sizeof(points) / sizeof(point_t);

	voronoi_init(&v);

	srand(37);
	for (size_t i = 0; i < n_points; i++)
	{
		points[i].x = ( (float) rand() / INT_MAX ) * 20;
		points[i].y = ( (float) rand() / INT_MAX ) * 20;
	}
	voronoi_points(&v, n_points, points);

	glutInit(&argc, argv);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Voronoi");
	glutSetCursor(GLUT_CURSOR_NONE);
	glutDisplayFunc (&cb_display);
	glutKeyboardFunc(&cb_keyboard);
	glInit();
	glutMainLoop();

	voronoi_exit(&v);
	return 0;
}
