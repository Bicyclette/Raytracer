#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "shader_light.hpp"

enum class DRAWING_MODE
{
	SOLID,
	WIREFRAME
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 tex)
	{
		position = pos;
		normal = norm;
		texCoords = tex;
	}
};

class Mesh
{
	public:

		Mesh(std::vector<Vertex> aVertices, std::vector<int> aIndices, Material m, std::string aName);
        ~Mesh();
		std::string getName();
		std::vector<Vertex> const& getVertices() const;
		std::vector<int> const& getIndices() const;
		Material & getMaterial();
		void bindVAO() const;
		void draw(Shader& s, bool instancing = false, int amount = 1, DRAWING_MODE mode = DRAWING_MODE::SOLID);

	private:

		GLuint vao;
		GLuint vbo;
		GLuint ebo;

		std::string name;
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		Material material;
};

#endif
