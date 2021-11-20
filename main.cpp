#include <iostream>
#include "engine/engine.hpp"
#include "engine/object.hpp"

#define WIDTH 800
#define HEIGHT 800

int main(int argc, char** argv) {

	SDL_Engine engine("3DEngine", WIDTH, HEIGHT);
	engine.run();

	return 0;
}
