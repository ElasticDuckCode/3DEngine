#include "object.hpp"
#include <cmath>

mesh load_mesh_obj(std::string f_name) {

	mesh object;
	std::ifstream f(f_name);

	if (!f.is_open()) {
		std::cerr << "ERROR: Failed to load object: " << f_name << std::endl;

	}

	std::vector<glm::vec3> verts;
	while (!f.eof()) {
		std::string s;
		std::stringstream ss;
		std::getline(f, s);
		ss << s;
		char junk;
		if (s[0] == 'v') { // add new vertex
			glm::vec3 v;
			ss >> junk >> v.x >> v.y >> v.z;
			verts.push_back(v);
		}
		if (s[0] == 'f') {
			int idx[3];
			ss >> junk >> idx[0] >> idx[1] >> idx[2];
			object.tri.push_back({verts[idx[0] - 1], verts[idx[1] - 1], verts[idx[2] - 1]});
		}
	}
	return object;
}

glm::vec3 midpoint(triangle tri) {
	glm::vec3 result = tri.vertex[0] + tri.vertex[1] + tri.vertex[2];
	result /= 3;
	return result;
}

glm::vec3 project_onto_vector(glm::vec3 v1, glm::vec3 v2) {
	float factor = glm::dot(v1, v2) / glm::dot(v2, v2);
	glm::vec3 result = factor * v2;
	return result;
}