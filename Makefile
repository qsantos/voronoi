CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -ansi -std=c99 -O3 -D_GNU_SOURCE
LDFLAGS = -O3 -lglut -lGL -lm
TARGETS = voronoi

voronoi: main.o lloyd.o voronoi.o geometry.o heap.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy $(TARGETS)
