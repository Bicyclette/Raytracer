#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "CL/cl.hpp"

class Light;
class PointLight;
class DirectionalLight;

class Shader
{
	public:

		Shader(const std::string & vertex_shader_file, const std::string & fragment_shader_file);
		Shader(const std::string & vertex_shader_file, const std::string & geometry_shader_file, const std::string & fragment_shader_file);
		~Shader();
		GLuint getId() const;
		void setInt(const std::string & name, int v) const;
		void setFloat(const std::string & name, float v) const;
		void setVec2f(const std::string & name, glm::vec2 v) const;
		void setVec3f(const std::string & name, glm::vec3 v) const;
		void setVec4f(const std::string & name, glm::vec4 v) const;
		void setMatrix(const std::string & name, glm::mat4 m) const;
		void setLighting(std::vector<std::shared_ptr<PointLight>> & pLights, std::vector<std::shared_ptr<DirectionalLight>> & dLights);
		void use() const;

	private:

		void compile(const char * vertex_shader_code, const char * fragment_shader_code);
		void compile(const char * vertex_shader_code, const char * geometry_shader_code, const char * fragment_shader_code);

		GLuint id;
};

enum class TEXTURE_TYPE
{
	DIFFUSE,
	SPECULAR,
	NORMAL
};

struct Texture
{
	GLuint id;
	TEXTURE_TYPE type;
	std::string path;
	unsigned char* img;
	int width;
	int height;
	int channels;

	Texture(){}

	Texture(GLuint aId, TEXTURE_TYPE aType, std::string aPath, unsigned char* data, int w, int h, int aChannels)
	{
		id = aId;
		type = aType;
		path = aPath;
		img = data;
		width = w;
		height = h;
		channels = aChannels;
	}
};

struct Material
{
    glm::vec4 color;
	float shininess;
	std::vector<Texture> textures; // [0] = diffuse, [1] = specular, [2] = normal
};

struct Texture createTexture(const std::string & texPath, TEXTURE_TYPE t, bool flip);

enum class LIGHT_TYPE
{
	POINT,
	DIRECTIONAL,
	SPOT
};

class Light
{
	public:

		Light(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, bool dynamic = false);
		virtual ~Light();
		virtual void draw() = 0;
		glm::vec3 getPosition();
		void setPosition(glm::vec3 pos);
		glm::vec3 getAmbientStrength();
		glm::vec3 getDiffuseStrength();
		glm::vec3 getSpecularStrength();
		float getBrightness();
		void setAmbientStrength(glm::vec3 c);
		void setDiffuseStrength(glm::vec3 c);
		void setSpecularStrength(glm::vec3 c);
		void setModelMatrix(glm::mat4 m);
		void setViewMatrix(glm::mat4 m);
		void setProjMatrix(glm::mat4 m);
		virtual LIGHT_TYPE getType() = 0;

	protected:

		float brightness;
		glm::vec3 ambientStrength;
		glm::vec3 diffuseStrength;
		glm::vec3 specularStrength;

		glm::vec3 position;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		
		GLuint vao;
		GLuint vbo;
		
		struct Texture icon;

		bool dynamicDraw;
};

typedef struct __attribute__ ((packed)) PointLight_t
{
	cl_float brightness;
	cl_float3 position;
	cl_int spCount;
} PointLight_t;

class PointLight : public Light
{
	public:

		PointLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, float aKc, float aKl, float aKq, float aRadius, int rsp, bool dynamic = false);
		virtual void draw() override;
		float getKc();
		float getKl();
		float getKq();
		std::vector<glm::vec3> & getSamplePoints();
		virtual LIGHT_TYPE getType() override;

	private:

		float kc;
		float kl;
		float kq;
		float radius;
		int rSamples;
		std::vector<glm::vec3> samplePoints;
		Shader shader;
};

class DirectionalLight : public Light
{
	public:

		DirectionalLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, glm::vec3 dir, bool dynamic = false);
		virtual void draw() override;
		glm::vec3 getDirection();
		void setDirection(glm::vec3 dir);
		virtual LIGHT_TYPE getType() override;

	private:

		glm::vec3 direction;
		Shader shaderIcon;
		Shader shaderDirection;
};

#endif
