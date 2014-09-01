CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -ansi -std=c99 -O3
LDFLAGS = -O3 -lglut -lGL -lm
TARGETS = voronoi

voronoi: main.o lloyd.o voronoi.o binbeach.o qsort_r.o geometry.o heap.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy
	@$(MAKE)

.PHONY: clean destroy rebuild
