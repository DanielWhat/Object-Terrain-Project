#g++ -Wall -o "bezier_surfaces" "bezier_surfaces.cpp" -lm -lGL -lGLU -lglut -lGLEW
CC = g++
CFLAGS = -Wall
FRAMEWORKS = -lGL -lGLU -lglut -lGLEW

all: Terrain.out

Terrain.o: Terrain.cpp
	$(CC) -c $(FLAGS) $< -o $@

Terrain.out: Terrain.o
	$(CC) $(CFLAGS) $^ -o $@ -lm $(FRAMEWORKS)
