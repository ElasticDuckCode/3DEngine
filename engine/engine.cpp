#include "engine.hpp"
#include "object.hpp"
#include <iostream>
#include <algorithm>

#define FOV 45.0
#define NEAR 0.1
#define FAR 1000.0

SDL_Engine::SDL_Engine(const char* title, int width, int height) {
    window = nullptr;
    renderer = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
		std::cerr << "SDL_Error: " << SDL_GetError() << std::endl;
	else
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
		                                       SDL_WINDOWPOS_UNDEFINED,
		                                       width, height,
		                                       SDL_WINDOW_SHOWN);

    if (!window) 
        std::cerr << "SDL_Error: " << SDL_GetError() << std::endl;
    else 
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
		std::cerr << "SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
	}
	this->w = width;
	this->h = height;
	this->proj = glm::perspective(glm::radians(FOV), double(this->w)/double(this->h), NEAR, FAR);

	// OpenGL Coordinates
	//     y
	//     |   
	//     .___x
	//    /
	//   z

	// SDL2 Coordinates
	//   .___x
	//   |
	//   y


	this->cam.pos = {0, 1, 10};
	this->cam.dir = {0, 0, -1};
	this->cam.up = {0, 1, 0};
}

SDL_Engine::~SDL_Engine() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

bool SDL_Engine::check_draw(triangle tri) {
	glm::vec3 v0 = tri.vertex[0];
	glm::vec3 v1 = tri.vertex[1];
	glm::vec3 v2 = tri.vertex[2];

	glm::vec3 x = v1 - v0;
	glm::vec3 y = v2 - v0;
	glm::vec3 z = glm::cross(x, y);
	return (glm::dot(z, this->cam.pos - v0) > 0.0);
}

std::vector<triangle> clip_plane(triangle tri, glm::vec3 n, glm::vec3 p) {
    std::vector<triangle> tris;
	glm::vec3 v0 = tri.vertex[0];
	glm::vec3 v1 = tri.vertex[1];
	glm::vec3 v2 = tri.vertex[2];

	// count inside
	float d0 = glm::dot(n, v0 - p);
	float d1 = glm::dot(n, v1 - p);
	float d2 = glm::dot(n, v2 - p);
	bool v0_in = (d0 >= 0);
	bool v1_in = (d1 >= 0);
	bool v2_in = (d2 >= 0);
	int count = v0_in + v1_in + v2_in;

	if (count == 3)	
		tris.push_back(tri);
	else if (count == 2) {
		triangle t1, t2;

		if (!v0_in) {
			glm::vec3 v01 = v1 + (d1/(d0-d1)) * (v1 - v0);
			glm::vec3 v02 = v2 + (d2/(d0-d2)) * (v2 - v0);

			t1.vertex[0] = v01;
			t1.vertex[1] = v1;
			t1.vertex[2] = v2;

			t2.vertex[0] = v02;
			t2.vertex[1] = v01;
			t2.vertex[2] = v2;
		}
		else if (!v1_in) {
			glm::vec3 v10 = v0 + (d0/(d1-d0)) * (v0 - v1);
			glm::vec3 v12 = v2 + (d2/(d1-d2)) * (v2 - v1);
			t1.vertex[0] = v0;
			t1.vertex[1] = v10;
			t1.vertex[2] = v2;
			t2.vertex[0] = v10;
			t2.vertex[1] = v12;
			t2.vertex[2] = v2;
		}
		else if (!v2_in) {
			glm::vec3 v20 = v0 + (d0/(d2-d0)) * (v0 - v2);
			glm::vec3 v21 = v1 + (d1/(d2-d1)) * (v1 - v2);
			t1.vertex[0] = v0;
			t1.vertex[1] = v1;
			t1.vertex[2] = v20;
			t2.vertex[0] = v20;
			t2.vertex[1] = v1;
			t2.vertex[2] = v21;
		}
		tris.push_back(t1);
		tris.push_back(t2);
	}
	else if (count == 1) {
		triangle t1;
		if (v0_in) {
			glm::vec3 v01 = v1 + (d1/(d0-d1)) * (v1 - v0);
			glm::vec3 v02 = v2 + (d2/(d0-d2)) * (v2 - v0);
			t1.vertex[0] = v0;
			t1.vertex[1] = v01;
			t1.vertex[2] = v02;
		}
		else if (v1_in) {
			glm::vec3 v10 = v0 + (d0/(d1-d0)) * (v0 - v1);
			glm::vec3 v12 = v2 + (d2/(d1-d2)) * (v2 - v1);
			t1.vertex[0] = v10;
			t1.vertex[1] = v1;
			t1.vertex[2] = v12;
		}
		else if (v2_in) {
			glm::vec3 v20 = v0 + (d0/(d2-d0)) * (v0 - v2);
			glm::vec3 v21 = v1 + (d1/(d2-d1)) * (v1 - v2);
			t1.vertex[0] = v20;
			t1.vertex[1] = v21;
			t1.vertex[2] = v2;
		}
		tris.push_back(t1);
	}
	return tris;
}

std::vector<triangle> SDL_Engine::clipZ(triangle tri) {

	std::vector<triangle> queue;
	std::vector<triangle> next_queue;

	queue.push_back(tri);

	// NEAR
	for (auto& t1 : queue) {
    	std::vector<triangle> next = clip_plane(t1, glm::vec3(0, 0, -1), glm::vec3(0, 0, -NEAR));
		for (auto& t2 : next) {
			next_queue.push_back(t2);
		}
	}
	queue = next_queue;
	next_queue = std::vector<triangle>();

	// FAR
	for (auto& t1 : queue) {
    	std::vector<triangle> next = clip_plane(t1, glm::vec3(0, 0, 1), glm::vec3(0, 0, -FAR));
		for (auto& t2 : next) {
			next_queue.push_back(t2);
		}
	}
	queue = next_queue;
	next_queue = std::vector<triangle>();

	return queue;
}

std::vector<triangle> SDL_Engine::clipXY(triangle tri) {

	std::vector<triangle> queue;
	std::vector<triangle> next_queue;

	queue.push_back(tri);

	// LEFT
	for (auto& t1 : queue) {
    	std::vector<triangle> next = clip_plane(t1, glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0));
		for (auto& t2 : next) {
			next_queue.push_back(t2);
		}
	}
	queue = next_queue;
	next_queue = std::vector<triangle>();

	// RIGHT
	for (auto& t1 : queue) {
    	std::vector<triangle> next = clip_plane(t1, glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0));
		for (auto& t2 : next) {
			next_queue.push_back(t2);
		}
	}
	queue = next_queue;
	next_queue = std::vector<triangle>();

	// TOP
	for (auto& t1 : queue) {
    	std::vector<triangle> next = clip_plane(t1, glm::vec3(0, -1, 0), glm::vec3(0, 1, 0));
		for (auto& t2 : next) {
			next_queue.push_back(t2);
		}
	}
	queue = next_queue;
	next_queue = std::vector<triangle>();

	// BOTTOM
	for (auto& t1 : queue) {
    	std::vector<triangle> next = clip_plane(t1, glm::vec3(0, 1, 0), glm::vec3(0, -1, 0));
		for (auto& t2 : next) {
			next_queue.push_back(t2);
		}
	}
	queue = next_queue;
	next_queue = std::vector<triangle>();


	return queue;
}


SDL_Engine& SDL_Engine::draw(triangle tri) {

	glm::vec3 v0 = tri.vertex[0];
	glm::vec3 v1 = tri.vertex[1];
	glm::vec3 v2 = tri.vertex[2];

	glm::mat4x4 view = glm::lookAt(
		this->cam.pos,
		this->cam.pos + this->cam.dir,
		this->cam.up
	);


	// illuminate before view shift
	glm::vec3 x = v1 - v0;
	glm::vec3 y = v2 - v0;
	glm::vec3 z = glm::cross(x, y);
	glm::vec3 illu = {0.0, 1.0, 1.0};
	illu = glm::normalize(illu);
	double level = (glm::dot(illu, glm::normalize(z)) + 1.0) * 0.5 - 0.01;

	v0 = view * glm::vec4(v0, 1.0);
	v1 = view * glm::vec4(v1, 1.0);
	v2 = view * glm::vec4(v2, 1.0);
	triangle to_draw = {v0, v1, v2};

	std::vector<triangle> clippedZ = this->clipZ(to_draw);

	for (auto& t : clippedZ) {

		v0 = t.vertex[0];
		v1 = t.vertex[1];
		v2 = t.vertex[2];

		glm::vec4 p0 = proj * glm::vec4(v0, 1.0);
		glm::vec4 p1 = proj * glm::vec4(v1, 1.0);
		glm::vec4 p2 = proj * glm::vec4(v2, 1.0);

		if (p0.w < 0 || p1.w < 0 || p2.w < 0) {
			return *this;
		}
		
		p0 /= p0.w;
		p1 /= p1.w;
		p2 /= p2.w;

		triangle to_draw2 = {p0, p1, p2};
		std::vector<triangle> clippedXY = this->clipXY(to_draw2);

		for (auto& t2 : clippedXY) {
			glm::vec3 p0 = t2.vertex[0];
			glm::vec3 p1 = t2.vertex[1];
			glm::vec3 p2 = t2.vertex[2];

			// SDL2 has flipped y
			p0.y *= -1;
			p1.y *= -1;
			p2.y *= -1;

			p0 += 1; p0 *= 0.5;
			p1 += 1; p1 *= 0.5;
			p2 += 1; p2 *= 0.5;

			p0.x *= this->w; p0.y *= this->h;
			p1.x *= this->w; p1.y *= this->h;
			p2.x *= this->w; p2.y *= this->h;

			level = fmax(level, 0.0);
			level = fmin(level, 1.0);

			filledTrigonRGBA(this->renderer, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, int(255 * level), int(255 * level), int(255 * level), 255);
			SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
			//SDL_RenderDrawLine(this->renderer, p0.x, p0.y, p1.x, p1.y);
			//SDL_RenderDrawLine(this->renderer, p1.x, p1.y, p2.x, p2.y);
			//SDL_RenderDrawLine(this->renderer, p2.x, p2.y, p0.x, p0.y);
		}

	}

	return *this;
}

SDL_Engine& SDL_Engine::draw(mesh m) {
	std::vector<triangle> to_draw;
	for (auto tri : m.tri) {
		// Check first if we need to draw so don't 
		// need to sort unwanted triangles.
		if (this->check_draw(tri)) {
			to_draw.push_back(tri);
		}
	}
	sort(to_draw.begin(), to_draw.end(), [&](triangle t1, triangle t2) {
		glm::vec3 m1 = project_onto_vector(midpoint(t1), this->cam.dir);
		glm::vec3 m2 = project_onto_vector(midpoint(t2), this->cam.dir);
		glm::vec3 pos = this->cam.pos;
		return glm::dot(m1-pos, m1-pos) > glm::dot(m2-pos, m2-pos);
	});
	for (auto& tri : to_draw) {
		this->draw(tri);
	}
	return *this;
}

SDL_Engine& SDL_Engine::run() {

    const char* fname = "resources/room2.obj";
	mesh object = load_mesh_obj(fname);

	std::cout << "[3DEngine] Loaded " << fname << " with " << object.tri.size() << " triangles." << std::endl;

	float move_speed = 2;
	bool W_DOWN = false;
	bool A_DOWN = false;
	bool S_DOWN = false;
	bool D_DOWN = false;
	
	bool UP_DOWN = false;
	bool DOWN_DOWN = false;
	bool LEFT_DOWN = false;
	bool RIGHT_DOWN = false;

	bool SPACE_DOWN = false;
	bool LSHIFT_DOWN = false;

	SDL_SetRelativeMouseMode(SDL_TRUE);
	float mouse_sense = 1e-2;
	bool FIRST_MOUSE = true;
	float xrel = 0.0f;
	float yrel = 0.0f;

	bool quit = false;

	uint64_t last_update = SDL_GetTicks();
	uint64_t current = SDL_GetTicks();
	float dT = (current - last_update) / 1000.0;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					if (e.key.keysym.sym == SDLK_ESCAPE)
						quit = true;
					if (e.key.keysym.sym == SDLK_w)
						W_DOWN = true;
					if (e.key.keysym.sym == SDLK_a)
						A_DOWN = true;
					if (e.key.keysym.sym == SDLK_s)
						S_DOWN = true;
					if (e.key.keysym.sym == SDLK_d)
						D_DOWN = true;
					if (e.key.keysym.sym == SDLK_LEFT)
						LEFT_DOWN = true;
					if (e.key.keysym.sym == SDLK_RIGHT)
						RIGHT_DOWN = true;
					if (e.key.keysym.sym == SDLK_UP)
						UP_DOWN = true;
					if (e.key.keysym.sym == SDLK_DOWN)
						DOWN_DOWN = true;
					if (e.key.keysym.sym == SDLK_SPACE)
						SPACE_DOWN = true;
					if (e.key.keysym.sym == SDLK_LSHIFT)
						LSHIFT_DOWN = true;
					break;
				case SDL_KEYUP:
					if (e.key.keysym.sym == SDLK_w)
						W_DOWN = false;
					if (e.key.keysym.sym == SDLK_a)
						A_DOWN = false;
					if (e.key.keysym.sym == SDLK_s)
						S_DOWN = false;
					if (e.key.keysym.sym == SDLK_d)
						D_DOWN = false;
					if (e.key.keysym.sym == SDLK_LEFT)
						LEFT_DOWN = false;
					if (e.key.keysym.sym == SDLK_RIGHT)
						RIGHT_DOWN = false;
					if (e.key.keysym.sym == SDLK_UP)
						UP_DOWN = false;
					if (e.key.keysym.sym == SDLK_DOWN)
						DOWN_DOWN = false;
					if (e.key.keysym.sym == SDLK_SPACE)
						SPACE_DOWN = false;
					if (e.key.keysym.sym == SDLK_LSHIFT)
						LSHIFT_DOWN = false;
					break;
				case SDL_MOUSEMOTION:
					if (!FIRST_MOUSE) {
						xrel = e.motion.xrel;
						yrel = e.motion.yrel;
					}
					else
						FIRST_MOUSE = false;
					break;
			}
		}

		// update physics timing
		current = SDL_GetTicks();
		dT = (current - last_update) / 1000.0f;
		last_update = current;

		if (W_DOWN && !S_DOWN) {
			glm::vec3 shift = {cam.dir.x, 0, cam.dir.z};
			shift = glm::normalize(shift);
			shift *= move_speed * dT;
			cam.pos += shift;
		}
		if (S_DOWN && !W_DOWN) {
			glm::vec3 shift = {cam.dir.x, 0, cam.dir.z};
			shift = glm::normalize(shift);
			shift *= move_speed * dT;
			cam.pos -= shift;
		}
		if (A_DOWN && !D_DOWN) {
			glm::vec3 shift = {cam.dir.x, 0, cam.dir.z};
			shift = glm::normalize(shift);
			shift = glm::cross(shift, cam.up);
			shift *= move_speed * dT;
			cam.pos -= shift;
		}
		if (D_DOWN && !A_DOWN) {
			glm::vec3 shift = {cam.dir.x, 0, cam.dir.z};
			shift = glm::normalize(shift);
			shift = glm::cross(shift, cam.up);
			shift *= move_speed * dT;
			cam.pos += shift;
		}

		if (xrel > 0.0 || xrel < 0.0) {
			cam.dir = glm::rotate(cam.dir, -mouse_sense * xrel, cam.up);
			xrel = 0.0;
		}
		if (yrel > 0.0 || yrel < 0.0) {
			glm::vec3 axis = glm::cross(cam.up, cam.dir);
			axis = glm::normalize(axis);
			cam.dir = glm::rotate(cam.dir, mouse_sense * yrel, axis);
			yrel = 0.0;
		}

		if (LEFT_DOWN && !RIGHT_DOWN) {
			cam.dir = glm::rotate(cam.dir, 0.01f, cam.up);
		}
		if (RIGHT_DOWN && !LEFT_DOWN) {
			cam.dir = glm::rotate(cam.dir, -0.01f, cam.up);
		}
		if (UP_DOWN && !DOWN_DOWN) {
			glm::vec3 axis = glm::cross(cam.up, cam.dir);
			axis = glm::normalize(axis);
			cam.dir = glm::rotate(cam.dir, 0.01f * dT, axis);
		}
		if (DOWN_DOWN && !UP_DOWN) {
			glm::vec3 axis = glm::cross(cam.up, cam.dir);
			axis = glm::normalize(axis);
			cam.dir = glm::rotate(cam.dir, -0.01f * dT, axis);
		}

		if (SPACE_DOWN && !LSHIFT_DOWN) {
			cam.pos += 2 * dT * cam.up;
		}
		if (LSHIFT_DOWN && !SPACE_DOWN) {
			cam.pos -= 2 * dT * cam.up;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		draw(object);

		SDL_RenderPresent(renderer);
	}
	return *this;
}