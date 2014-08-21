Voronoi
=======

This programs shows the computing of a Voronoi diagram for a set of
points chosen at random. It is intended to be used as a library for
larger projects.

Compiling
---------

To compile you will need to install glut for development. For instance
with the Debian package `freeglut3-dev`.

Keybindings
-----------

You can use `Escape` to exit at any time. A number of keybindings are
available to go through the animation:

* `Space` pushes the front line by a tiny amount
* `G` proceeds 1 step
* `F` proceeds 10 steps
* `D` proceeds 100 steps
* `S` proceeds 1000 steps

Additionally, you can use `Enter` to finish the diagram and `L` to trigger
a relaxation from Lloyd's algorithm (it finishes the diagram and start
a new one with the centroids from the previous one as the initial set
of points).

Licence
-------

This program is distributed under the GPL licence (see
[LICENCE.md](LICENCE.md) file). The credits for markdown formatting goes
to https://github.com/IQAndreas/markdown-licenses
