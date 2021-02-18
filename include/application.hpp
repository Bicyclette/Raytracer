#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <mutex>
#include <cmath>
#include <limits>
#include "scene.hpp"
#include "graphics.hpp"
#include "window.hpp"
#include "stb_image_write.h"
#include "clprogram.hpp"

struct RenderData
{
	void* app;
	std::shared_ptr<WindowManager> window;

	RenderData()
	{
		app = nullptr;
	}
};

struct PixelShade
{
	glm::vec3 color;
	float diffuse;
	float specular;

	PixelShade(){}

	PixelShade(glm::vec3 c, float diff, float spec)
	{
		color = c;
		diffuse = diff;
		specular = spec;
	}
};

namespace RT
{
	void computePrimaryRay(int& x, int& y, const int& scrX, const int& scrY, std::shared_ptr<Camera> & cam, glm::vec3 & primRay);
	float computeShadowFactor(const glm::vec3 & fragPos, const glm::vec3 & fragNormal, std::vector<glm::vec3> & lightSamples, std::vector<std::shared_ptr<Mesh>> & meshes);
	float computeDiffuse(const glm::vec3 & pHit, const glm::vec3 & nHit, const glm::vec3 & lightPosition);
	float computeSpecular(const glm::vec3 & pHit, const glm::vec3 & nHit, const glm::vec3 & lightPosition, const glm::vec3 & primRay, const float & shininess);
	struct PixelShade computePixelShade(std::shared_ptr<Mesh> & m, glm::vec3 & fragPos, glm::vec3 & fragNormal, glm::vec2 & fragTexCoords, glm::vec3 & lPos, glm::vec3 & primRay);
	bool intersect(std::shared_ptr<Mesh> & mesh, glm::vec3 & rayOrigin, glm::vec3 & rayDir, glm::vec3 & pHit, glm::vec3 & nHit, glm::vec2 & texCoords);
	bool rayTriangleIntersect(const glm::vec3 & origin, const glm::vec3 & dir, glm::vec3 & v0, glm::vec3 & v1, glm::vec3 & v2, float & t, float & u, float & v);
	float distance(const glm::vec3 & v1, const glm::vec3 & v2);
	int raytrace(int id, int nbThr, RenderData* rtData, unsigned char* img);
};

class Application
{
	public:

		Application(int clientWidth, int clientHeight);
		void drawScene(int index, int width, int height, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool debug = false);
		void renderScene(std::shared_ptr<WindowManager> client, bool useGPU = false);
		void setActiveScene(int index);
		int getActiveScene();
		void resizeScreen(int clientWidth, int clientHeight);
		void updateSceneActiveCameraView(int index, const std::bitset<16> & inputs, std::array<int, 3> & mouse, float delta);
		std::vector<std::shared_ptr<Scene>> & getScenes();

	private:

		int activeScene;
		RenderData rtData;
		CLProgram clRaytrace;
		std::vector<std::shared_ptr<Scene>> scenes;
		std::unique_ptr<Graphics> graphics;
		
		void directionalShadowPass(int index, DRAWING_MODE mode = DRAWING_MODE::SOLID);
		void omnidirectionalShadowPass(int index, DRAWING_MODE mode = DRAWING_MODE::SOLID);
		void colorMultisamplePass(int index, int width, int height, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool debug = false);
};

#endif
