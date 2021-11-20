CC=clang++
#CFLAGS=`sdl2-config --cflags --libs`

run: main.cpp engine/engine.cpp engine/object.cpp
	$(CC) main.cpp engine/engine.cpp engine/object.cpp -o run -lSDL2 -lSDL2_gfx

clean:
	rm run

