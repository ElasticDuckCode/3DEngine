#ifndef OBJECT_HPP
#define OBJECT_HPP 

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

struct triangle {
	glm::vec3 vertex[3];
};

struct mesh {
	std::vector<triangle> tri;
};

glm::vec3 midpoint(triangle tri);
glm::vec3 project_onto_vector(glm::vec3 v1, glm::vec3 v2);
mesh load_mesh_obj(std::string f_name);

#endif
