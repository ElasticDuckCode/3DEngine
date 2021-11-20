#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "object.hpp"

/*
	Jake Millhiser
	2021-11-06

	engine.cpp:

		Define class to contain the SDL2 initalization procedure
		and game loop so that in the case of any failure
		the destructor will properly close and free memory.
*/

struct Camera {
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 up;
};

class SDL_Engine {
protected:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event e;
	int w;
	int h;
	glm::mat4x4 proj;
	Camera cam;
public:
	SDL_Engine(const char* title, int width, int height);
	~SDL_Engine();
	SDL_Engine& run();

	// drawing functions
	bool check_draw(triangle tri);
	std::vector<triangle> clipXY(triangle tri);
	std::vector<triangle> clipZ(triangle tri);
	SDL_Engine& draw(triangle tri);
	SDL_Engine& draw(mesh m);
};

#endif
