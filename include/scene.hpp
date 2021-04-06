#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include "skybox.hpp"
#include "object.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "grid_axis.hpp"

class Scene
{
	public:

		Scene(std::string pName);
		void addObject(std::string filePath, glm::mat4 aModel = glm::mat4(1.0f), const std::vector<glm::mat4> & instanceModel = std::vector<glm::mat4>());
		void addCamera(float aspectRatio, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 target = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float fov = 45.0f, float near = 0.1f, float far = 100.0f);
		void addPointLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, float aKc, float aKl, float aKq, float aRadius, int rsp, bool dynamic = false);
		void addDirectionalLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, glm::vec3 dir, bool dynamic = false);
		void setSkybox(std::vector<std::string> & textures, bool flip);
		void setGridAxis(int gridDim);
		void setActiveCamera(int index);
		void updateCameraPerspective(float aspectRatio);
		std::vector<std::shared_ptr<Camera>> & getCameras();
		std::shared_ptr<Camera> & getActiveCamera();
		std::string & getName();
		std::vector<std::shared_ptr<Object>> & getObjects();
		void draw(Shader & shader, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool debug = false);
		std::vector<std::shared_ptr<PointLight>> & getPLights();
		std::vector<std::shared_ptr<DirectionalLight>> & getDLights();

	private:

		std::string name;

		std::shared_ptr<Camera> activeCamera;
		std::vector<std::shared_ptr<Camera>> cameras;

		std::vector<std::shared_ptr<Object>> objects;
		
		std::vector<std::shared_ptr<PointLight>> pLights;
		std::vector<std::shared_ptr<DirectionalLight>> dLights;
		
		std::unique_ptr<Skybox> sky;

		std::unique_ptr<GridAxis> gridAxis;
};

#endif
