#g++ -Wall -o "bezier_surfaces" "bezier_surfaces.cpp" -lm -lGL -lGLU -lglut -lGLEW
CC = g++
CFLAGS = -Wall
FRAMEWORKS = -lGL -lGLU -lglut -lGLEW

all: bezier_surfaces.out

bezier_surfaces.o: bezier_surfaces.cpp
	$(CC) -c $(FLAGS) $< -o $@

bezier_surfaces.out: bezier_surfaces.o
	$(CC) $(CFLAGS) $^ -o $@ -lm $(FRAMEWORKS)
