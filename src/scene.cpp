#include "scene.hpp"

Scene::Scene(std::string pName) : name(pName) {}

void Scene::addObject(std::string filePath, glm::mat4 aModel, const std::vector<glm::mat4> & instanceModel)
{
	objects.push_back(std::make_shared<Object>(filePath, aModel));

	if(instanceModel.size() > 0)
	{
		objects.at(objects.size() - 1)->setInstancing(instanceModel);
	}
}

void Scene::addCamera(float aspectRatio, glm::vec3 pos, glm::vec3 target, glm::vec3 up, float fov, float near, float far)
{
	cameras.push_back(std::make_shared<Camera>(aspectRatio, pos, target, up, fov, near, far));
}

void Scene::addPointLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, float aKc, float aKl, float aKq, float aRadius, int rsp, bool dynamic)
{
	pLights.push_back(std::make_shared<PointLight>(pos, amb, diff, spec, bright, aKc, aKl, aKq, aRadius, rsp, dynamic));
	pLights.at(pLights.size() - 1)->setModelMatrix(glm::mat4(1.0f));
}

void Scene::addDirectionalLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float bright, glm::vec3 dir, bool dynamic)
{
	dLights.push_back(std::make_shared<DirectionalLight>(pos, amb, diff, spec, bright, dir, dynamic));
	dLights.at(dLights.size() - 1)->setModelMatrix(glm::mat4(1.0f));
}

void Scene::setSkybox(std::vector<std::string> & textures, bool flip)
{
	sky = std::make_unique<Skybox>(textures, flip);
}

void Scene::setGridAxis(int gridDim)
{
	gridAxis = std::make_unique<GridAxis>(8);
}

void Scene::setActiveCamera(int index)
{
	activeCamera = cameras.at(index);
}

void Scene::updateCameraPerspective(float aspectRatio)
{
	for(int i{0}; i < cameras.size(); ++i)
	{
		std::shared_ptr<Camera> cam = cameras.at(i);
		cameras.at(i)->setProjection(aspectRatio, cam->getNearPlane(), cam->getFarPlane());
	}
}

std::vector<std::shared_ptr<Camera>> & Scene::getCameras()
{
	return cameras;
}

std::shared_ptr<Camera> & Scene::getActiveCamera()
{
	return activeCamera;
}

std::string & Scene::getName()
{
	return name;
}

std::vector<std::shared_ptr<Object>> & Scene::getObjects()
{
	return objects;
}

void Scene::draw(Shader & shader, DRAWING_MODE mode, bool debug)
{
	if(debug)
	{
		if(gridAxis)
		{
			gridAxis->draw(activeCamera->getViewMatrix(), activeCamera->getProjectionMatrix());
		}

		for(int i{0}; i < pLights.size(); ++i)
		{
			pLights[i]->setViewMatrix(activeCamera->getViewMatrix());
			pLights[i]->setProjMatrix(activeCamera->getProjectionMatrix());
			pLights[i]->draw();
		}
		for(int i{0}; i < dLights.size(); ++i)
		{
			dLights[i]->setViewMatrix(activeCamera->getViewMatrix());
			dLights[i]->setProjMatrix(activeCamera->getProjectionMatrix());
			dLights[i]->draw();
		}
	}
	
	for(int i{0}; i < objects.size(); ++i)
	{
		objects[i]->draw(shader, mode);
	}

	if(sky)
	{
		sky->draw(activeCamera->getViewMatrix(), activeCamera->getProjectionMatrix());
	}
}

std::vector<std::shared_ptr<PointLight>> & Scene::getPLights()
{
	return pLights;
}

std::vector<std::shared_ptr<DirectionalLight>> & Scene::getDLights()
{
	return dLights;
}
