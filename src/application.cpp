#include "application.hpp"
#include <glm/gtx/rotate_vector.hpp>

Application::Application(int clientWidth, int clientHeight) :
	activeScene(-1),
	clRaytrace("../clKernels/raytracing.cl"),
	graphics(std::make_unique<Graphics>(clientWidth, clientHeight))
{
	// window aspect ratio
	float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);	
	
	// create minecraft scene
	scenes.push_back(std::make_shared<Scene>("minecraft"));
	scenes.at(scenes.size()-1)->addCamera(aspectRatio, glm::vec3(0.0f, 1.5f, 4.0f), glm::vec3(0.0f), glm::normalize(glm::vec3(0.0f, 4.0f, -1.5f)), 60.0f, 0.1f, 100.0f );
	scenes.at(scenes.size()-1)->setActiveCamera(0);
	scenes.at(scenes.size()-1)->addPointLight(glm::vec3(3.0f, 4.0f, 5.0f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 1.0f, 0.045f, 0.0075f, 0.5f, 5);
	scenes.at(scenes.size()-1)->addObject("../assets/minecraft/minecraft.obj", glm::mat4(1.0f));
	scenes.at(scenes.size()-1)->setGridAxis(8);

	// create angel scene
	scenes.push_back(std::make_shared<Scene>("angel"));
	scenes.at(scenes.size()-1)->addCamera(aspectRatio, glm::vec3(0.0f, 1.5f, 4.0f), glm::vec3(0.0f), glm::normalize(glm::vec3(0.0f, 4.0f, -1.5f)), 60.0f, 0.1f, 100.0f );
	scenes.at(scenes.size()-1)->setActiveCamera(0);
	scenes.at(scenes.size()-1)->addPointLight(glm::vec3(5.0f, 13.0f, 5.0f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 1.0f, 0.045f, 0.0075f, 0.5f, 2);
	scenes.at(scenes.size()-1)->addObject("../assets/angel/angel.obj", glm::mat4(1.0f));
	scenes.at(scenes.size()-1)->setGridAxis(8);
/*
	// create boots scene
	scenes.push_back(std::make_shared<Scene>("boots"));
	scenes.at(scenes.size()-1)->addCamera(aspectRatio, glm::vec3(0.0f, 1.5f, 4.0f), glm::vec3(0.0f), glm::normalize(glm::vec3(0.0f, 4.0f, -1.5f)), 60.0f, 0.1f, 100.0f );
	scenes.at(scenes.size()-1)->setActiveCamera(0);
	scenes.at(scenes.size()-1)->addPointLight(glm::vec3(3.0f, 4.0f, 5.0f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 1.0f, 0.045f, 0.0075f, 0.5f, 3);
	scenes.at(scenes.size()-1)->addObject("../assets/boots/boots.obj", glm::mat4(1.0f));
	scenes.at(scenes.size()-1)->setGridAxis(8);
*/

	// create street light scene
	scenes.push_back(std::make_shared<Scene>("street light"));
	scenes.at(scenes.size()-1)->addCamera(aspectRatio, glm::vec3(0.0f, 4.0f, 10.0f), glm::vec3(0.0f), glm::normalize(glm::vec3(0.0f, 4.0f, -1.5f)), 60.0f, 0.1f, 100.0f );
	scenes.at(scenes.size()-1)->setActiveCamera(0);
	scenes.at(scenes.size()-1)->addPointLight(glm::vec3(0.0f, 12.0f, 0.0f), glm::vec3(0.025f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 1.0f, 0.045f, 0.0075f, 0.5f, 2);
	scenes.at(scenes.size()-1)->addObject("../assets/street_light/street_light.obj", glm::mat4(1.0f));
	scenes.at(scenes.size()-1)->setGridAxis(8);
}

void Application::drawScene(int index, int width, int height, DRAWING_MODE mode, bool debug)
{
	if(index < scenes.size())
	{
		if(graphics->getShadowQuality() != SHADOW_QUALITY::OFF)
		{
			graphics->getBlinnPhongShader().use();
			graphics->getBlinnPhongShader().setInt("shadowOn", 1);

			// SHADOW PASS : directional & spot light sources
			directionalShadowPass(index, mode);
			
			// SHADOW PASS : point light sources
			omnidirectionalShadowPass(index, mode);

			// COLOR PASS : multisampling
			colorMultisamplePass(index, width, height, mode, debug);

			// blit to normal framebuffer (resolve multisampling)
			graphics->getNormalFBO()->bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			graphics->getMultisampleFBO()->blitFramebuffer(graphics->getNormalFBO(), width, height);

			// bind to default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// draw post processing quad
			graphics->getQuadMesh()->draw(graphics->getPostProcessingShader());
		}
		else
		{
			graphics->getBlinnPhongShader().use();
			graphics->getBlinnPhongShader().setInt("shadowOn", 0);
			
			// render to multisample framebuffer
			glViewport(0, 0, width, height);
			graphics->getMultisampleFBO()->bind();
			
			// draw scene
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			graphics->getBlinnPhongShader().setInt("renderingDepth", 0);
			graphics->getBlinnPhongShader().setInt("omniDepthRendering", 0);
			graphics->getBlinnPhongShader().setVec3f("cam.viewPos", scenes.at(index)->getActiveCamera()->getPosition());
			graphics->getBlinnPhongShader().setMatrix("view", scenes.at(index)->getActiveCamera()->getViewMatrix());
			graphics->getBlinnPhongShader().setMatrix("proj", scenes.at(index)->getActiveCamera()->getProjectionMatrix());
			graphics->getBlinnPhongShader().setLighting(scenes.at(index)->getPLights(), scenes.at(index)->getDLights());
			scenes.at(index)->draw(graphics->getBlinnPhongShader(), mode, debug);

			// blit to normal framebuffer (resolve multisampling)
			graphics->getNormalFBO()->bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			graphics->getMultisampleFBO()->blitFramebuffer(graphics->getNormalFBO(), width, height);

			// bind to default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// draw post processing quad
			graphics->getQuadMesh()->draw(graphics->getPostProcessingShader());
		}
	}
	else
	{
		std::cerr << "Error: wrong scene index supplied for draw command.\n";
	}
}

void Application::resizeScreen(int clientWidth, int clientHeight)
{
	// window aspect ratio
	float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);	

	#pragma omp for
	for(int i{0}; i < scenes.size(); ++i)
	{
		scenes.at(i)->getActiveCamera()->updateProjectionMatrix(clientWidth, clientHeight);
	}

	graphics->resizeScreen(clientWidth, clientHeight);
}

void Application::updateSceneActiveCameraView(int index, const std::bitset<16> & inputs, std::array<int, 3> & mouse, float delta)
{
	if(index < scenes.size())
	{
		scenes.at(index)->getActiveCamera()->updateViewMatrix(inputs, mouse, delta);
	}
}

std::vector<std::shared_ptr<Scene>> & Application::getScenes()
{
	return scenes;
}

void Application::directionalShadowPass(int index, DRAWING_MODE mode)
{
	glViewport(0, 0, static_cast<int>(graphics->getShadowQuality()), static_cast<int>(graphics->getShadowQuality()));
	graphics->getShadowMappingShader().use();
	graphics->getShadowMappingShader().setInt("computeWorldPos", 0);
	graphics->getShadowMappingShader().setInt("omniDepthRendering", 0);
	graphics->getShadowMappingShader().setInt("omnilightFragDepth", 0);
	graphics->getShadowMappingShader().setMatrix("proj", graphics->getOrthoProjection());
			
	// render directional depth maps
	for(int i{0}; i < scenes.at(index)->getDLights().size(); ++i)
	{
		graphics->getStdDepthFBO(i)->bind();
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::vec3 lightPosition = scenes.at(index)->getDLights().at(i)->getPosition();
		glm::vec3 lightTarget = lightPosition + scenes.at(index)->getDLights().at(i)->getDirection();
		glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));

		graphics->getShadowMappingShader().setMatrix("view", lightView);

		// draw scene
		scenes.at(index)->draw(graphics->getShadowMappingShader(), mode);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::omnidirectionalShadowPass(int index, DRAWING_MODE mode)
{
	glViewport(0, 0, static_cast<int>(graphics->getShadowQuality()), static_cast<int>(graphics->getShadowQuality()));
	graphics->getShadowMappingShader().use();
	graphics->getShadowMappingShader().setInt("computeWorldPos", 1);
	graphics->getShadowMappingShader().setInt("omniDepthRendering", 1);
	graphics->getShadowMappingShader().setInt("omnilightFragDepth", 1);
	
	// render omnidirectional depth maps
	std::vector<glm::mat4> omnilightViews;
	for(int i{0}; i < scenes.at(index)->getPLights().size(); ++i)
	{
		graphics->getOmniDepthFBO(i)->bind();
		glClear(GL_DEPTH_BUFFER_BIT);

		// (proj * view)
		glm::vec3 lightPosition = scenes.at(index)->getPLights().at(i)->getPosition();
		omnilightViews.push_back(graphics->getOmniPerspProjection() * glm::lookAt(lightPosition, lightPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		omnilightViews.push_back(graphics->getOmniPerspProjection() * glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		omnilightViews.push_back(graphics->getOmniPerspProjection() * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		omnilightViews.push_back(graphics->getOmniPerspProjection() * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		omnilightViews.push_back(graphics->getOmniPerspProjection() * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		omnilightViews.push_back(graphics->getOmniPerspProjection() * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		graphics->getShadowMappingShader().setVec3f("lightPosition", lightPosition);
		for(int j{0}; j < 6; ++j)
			graphics->getShadowMappingShader().setMatrix("omnilightViews[" + std::to_string(j) + "]", omnilightViews.at(j));

		// draw scene
		scenes.at(index)->draw(graphics->getShadowMappingShader(), mode);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::colorMultisamplePass(int index, int width, int height, DRAWING_MODE mode, bool debug)
{
	// render to multisample framebuffer
	glViewport(0, 0, width, height);
	graphics->getMultisampleFBO()->bind();

	// draw scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	graphics->getBlinnPhongShader().use();
	graphics->getBlinnPhongShader().setInt("pointLightCount", scenes.at(index)->getPLights().size());
	graphics->getBlinnPhongShader().setVec3f("cam.viewPos", scenes.at(index)->getActiveCamera()->getPosition());
	graphics->getBlinnPhongShader().setMatrix("view", scenes.at(index)->getActiveCamera()->getViewMatrix());
	graphics->getBlinnPhongShader().setMatrix("proj", scenes.at(index)->getActiveCamera()->getProjectionMatrix());
	graphics->getBlinnPhongShader().setLighting(scenes.at(index)->getPLights(), scenes.at(index)->getDLights());

	// set shadow maps (point first, dir second and spot last)
	int nbPLights = scenes.at(index)->getPLights().size();
	int nbDLights = scenes.at(index)->getDLights().size();
	
	int textureOffset{3};
	
	for(int i{0}; i < scenes.at(index)->getPLights().size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + textureOffset);
		glBindTexture(GL_TEXTURE_CUBE_MAP, graphics->getOmniDepthFBO(i)->getAttachments().at(0).id);
		graphics->getBlinnPhongShader().setInt("omniDepthMap[" + std::to_string(i) + "]", textureOffset);
		graphics->getBlinnPhongShader().setMatrix("light[" + std::to_string(i) + "].lightSpaceMatrix", glm::mat4(1.0f));
		textureOffset++;
	}
			
	int depthMapIndex{0};
	for(int i{0}; i < scenes.at(index)->getDLights().size(); ++i)
	{
		glm::vec3 lightPosition = scenes.at(index)->getDLights().at(i)->getPosition();
		glm::vec3 lightTarget = lightPosition + scenes.at(index)->getDLights().at(i)->getDirection();
		glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));

		glActiveTexture(GL_TEXTURE0 + textureOffset);
		glBindTexture(GL_TEXTURE_2D, graphics->getStdDepthFBO(depthMapIndex)->getAttachments().at(0).id);
		graphics->getBlinnPhongShader().setInt("depthMap[" + std::to_string(depthMapIndex) + "]", textureOffset);
		graphics->getBlinnPhongShader().setMatrix("light[" + std::to_string(i + nbPLights) + "].lightSpaceMatrix", graphics->getOrthoProjection() * lightView);
		depthMapIndex++;
		textureOffset++;
	}

	scenes.at(index)->draw(graphics->getBlinnPhongShader(), mode, debug);
}

void Application::setActiveScene(int index)
{
	activeScene = index;
}

int Application::getActiveScene()
{
	return activeScene;
}

void Application::renderScene(std::shared_ptr<WindowManager> client, bool useGPU)
{
	SDL_SetWindowInputFocus(client->getRenderViewPtr());
	SDL_ShowWindow(client->getRenderViewPtr());

	// clear window
	SDL_SetRenderDrawColor(client->getRendererPtr(), 255, 255, 255, 255);
	SDL_RenderClear(client->getRendererPtr());

	// get max number of supported threads on the machine
	int thrCount = std::thread::hardware_concurrency();

	// width and height of render view
	int width = client->getWidth();
	int height = client->getHeight();
	unsigned char* img = new unsigned char[3*width*height];

	if(useGPU)
	{
		const int N = width*height;

		// get camera ##########
		// #####################
		std::shared_ptr<Camera> cam = getScenes().at(activeScene)->getActiveCamera();
		glm::vec3 camPos = cam->getPosition();
		glm::vec3 camTarget = cam->getTarget();
		glm::vec3 camRight = cam->getRight();
		glm::vec3 camUp = cam->getUp();
		float camFov = cam->getFov();
		
		std::unique_ptr<Camera_t> camData = std::make_unique<Camera_t>();
		camData->position = (cl_float3){camPos.x, camPos.y, camPos.z};
		camData->target = (cl_float3){camTarget.x, camTarget.y, camTarget.z};
		camData->right = (cl_float3){camRight.x, camRight.y, camRight.z};
		camData->up = (cl_float3){camUp.x, camUp.y, camUp.z};
		camData->fov = (cl_float){camFov};

		// get point light ##########
		// ##########################
		std::vector<std::shared_ptr<PointLight>> pLights = getScenes().at(activeScene)->getPLights();
		std::shared_ptr<PointLight> light = pLights.at(0);
		float brightness = light->getBrightness();
		glm::vec3 lightPosition = light->getPosition();
		std::vector<glm::vec3> samplePoints = light->getSamplePoints();
		std::unique_ptr<cl_float3[]> lightSamplePoints = std::make_unique<cl_float3[]>(samplePoints.size());
		for(int i{0}; i < samplePoints.size(); ++i)
		{
			glm::vec3 sample = samplePoints.at(i);
			lightSamplePoints[i] = (cl_float3){sample.x, sample.y, sample.z};
		}
		int spCount = samplePoints.size();

		std::unique_ptr<PointLight_t> pl = std::make_unique<PointLight_t>();
		pl->brightness = (cl_float){brightness};
		pl->position = (cl_float3){lightPosition.x, lightPosition.y, lightPosition.z};
		pl->spCount = (cl_int){spCount};

		// get objects of the scene
		std::vector<std::shared_ptr<Object>> objs = getScenes().at(activeScene)->getObjects();

		// get all meshes of the scene
		std::vector<std::shared_ptr<Mesh>> meshes;
		for(int i{0}; i < objs.size(); ++i)
		{
			std::vector<std::shared_ptr<Mesh>> current = objs.at(i)->getMeshes();
			meshes.insert(meshes.end(), current.begin(), current.end());
		}
		
		// get meshes data ##########
		// ##########################
		const int meshCount = meshes.size();
		std::vector<float> vertices;
		int vertexStride = 8;
		std::vector<int> verticesCount;
		std::vector<int> indices;
		std::vector<int> indicesCount;
		std::vector<float> colorShininess;
		std::vector<int> texturesCount;
		std::vector<unsigned char> texData;
		std::vector<int> texAttributes;
		for(int i{0}; i < meshes.size(); ++i)
		{
			std::shared_ptr<Mesh> mesh = meshes.at(i);
			std::vector<Vertex> meshVertices = mesh->getVertices();
			std::vector<int> meshIndices = mesh->getIndices();
			Material mat = mesh->getMaterial();
			std::vector<Texture> textures = mat.textures;

			// vertices
			for(int j{0}; j < meshVertices.size(); ++j)
			{
				glm::vec3 pos = meshVertices.at(j).position;
				glm::vec3 norm = meshVertices.at(j).normal;
				glm::vec2 texCoords = meshVertices.at(j).texCoords;

				vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
				vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);
				vertices.push_back(texCoords.x); vertices.push_back(texCoords.y);
			}
			verticesCount.push_back(meshVertices.size());

			// indices
			indices.insert(indices.end(), meshIndices.begin(), meshIndices.end());
			indicesCount.push_back(meshIndices.size());

			// material
			glm::vec4 color = mat.color;
			colorShininess.push_back(color.r);
			colorShininess.push_back(color.g);
			colorShininess.push_back(color.b);
			colorShininess.push_back(color.a);
			colorShininess.push_back(mat.shininess);

			texturesCount.push_back(textures.size());
			for(int j{0}; j < textures.size(); ++j)
			{
				unsigned char* img = textures.at(j).img;
				texAttributes.push_back(textures.at(j).width);
				texAttributes.push_back(textures.at(j).height);
				texAttributes.push_back(textures.at(j).channels);

				for(int p{0}; p < (textures.at(j).width * textures.at(j).height * textures.at(j).channels); ++p)
				{
					texData.push_back(img[p]);
				}
			}
		}

		// set buffers on device
		cl::Context context = clRaytrace.getContext();
		cl::Buffer imgBuffer(context, CL_MEM_READ_WRITE, 3*width*height*sizeof(unsigned char));
		cl::Buffer camDataBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(struct Camera_t), camData.get());
		cl::Buffer plBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(struct PointLight_t), pl.get());
		cl::Buffer lightSamplePointsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, samplePoints.size() * sizeof(cl_float3), lightSamplePoints.get());
		cl::Buffer verticesBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, vertices.size() * sizeof(cl_float), vertices.data());
		cl::Buffer verticesCountBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, verticesCount.size() * sizeof(cl_int), verticesCount.data());
		cl::Buffer indicesBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, indices.size() * sizeof(cl_int), indices.data());
		cl::Buffer indicesCountBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, indicesCount.size() * sizeof(cl_int), indicesCount.data());
		cl::Buffer colorShininessBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, colorShininess.size() * sizeof(cl_float), colorShininess.data());
		cl::Buffer texturesCountBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, texturesCount.size() * sizeof(cl_int), texturesCount.data());
		cl::Buffer texDataBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, texData.size() * sizeof(unsigned char), texData.data());
		cl::Buffer texAttributesBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, texAttributes.size() * sizeof(cl_int), texAttributes.data());

		// set kernel parameters
		cl::Kernel kernel = clRaytrace.getKernel();
		kernel.setArg(0, N);
		kernel.setArg(1, imgBuffer);
		kernel.setArg(2, width);
		kernel.setArg(3, height);
		kernel.setArg(4, camDataBuffer);
		kernel.setArg(5, plBuffer);
		kernel.setArg(6, lightSamplePointsBuffer);
		kernel.setArg(7, meshCount);
		kernel.setArg(8, verticesBuffer);
		kernel.setArg(9, (cl_int){8});
		kernel.setArg(10, verticesCountBuffer);
		kernel.setArg(11, indicesBuffer);
		kernel.setArg(12, indicesCountBuffer);
		kernel.setArg(13, colorShininessBuffer);
		kernel.setArg(14, texturesCountBuffer);
		kernel.setArg(15, texDataBuffer);
		kernel.setArg(16, texAttributesBuffer);
		kernel.setArg(17, (cl_int){3});
		
		// launch kernel on the compute device
		clRaytrace.getCommandQueue().enqueueNDRangeKernel(kernel, cl::NullRange, N, cl::NullRange);

		// get result back to host
		clRaytrace.getCommandQueue().enqueueReadBuffer(imgBuffer, CL_TRUE, 0, 3*width*height*sizeof(unsigned char), img);
	}
	else
	{
		// prepare data
		rtData.app = static_cast<void*>(this);
		rtData.window = client;

		// start rendering on a pool of threads
		std::vector<std::unique_ptr<std::thread>> threads;
		threads.reserve(thrCount);

		for(int i{0}; i < thrCount; ++i)
		{
			threads.push_back(std::make_unique<std::thread>(RT::raytrace, i, thrCount, &rtData, img));
			threads.at(i)->join();
		}
	}

	// print result	
	std::cout << "Raytracing finished." << std::endl;
	std::cout << "Printing result" << std::endl;
	for(int i{0}; i < width; ++i)
	{
		for(int j{0}; j < height; ++j)
		{
			int index; 
			if(j >= 1)
				index = 3 * ((j - 1) * width + i);
			else
				index = 3 * i;

			int imgR = img[index];
			int imgG = img[index+1];
			int imgB = img[index+2];

			SDL_SetRenderDrawColor(client->getRendererPtr(), imgR, imgG, imgB, 255);
			SDL_RenderDrawPoint(client->getRendererPtr(), i, (height-1) - j);
		}
	}
	//SDL_RenderPresent(client->getRendererPtr());
	
	while(client->getShowRenderView())
	{
		client->checkEvents();

		if(client->getUserInputs().test(6) && client->getUserInputs().test(8))
		{
			// save image and hide view
			stbi_flip_vertically_on_write(1);
			stbi_write_jpg("../assets/render/img.jpg", width, height, 3, img, 100);
		}

		client->resetEvents();
	}

	// focus back to main window and hide render view
	SDL_SetWindowInputFocus(client->getWindowPtr());
	SDL_HideWindow(client->getRenderViewPtr());
}

int RT::raytrace(int id, int nbThr, RenderData* rtData, unsigned char* img)
{
	Application* app = static_cast<Application*>(rtData->app);
	std::shared_ptr<WindowManager> client = rtData->window;

	// get active scene index
	int activeScene = app->getActiveScene();
	
	// get camera
	std::shared_ptr<Camera> cam = app->getScenes().at(activeScene)->getActiveCamera();
	glm::vec3 camPos = cam->getPosition();

	// get point light
	std::vector<std::shared_ptr<PointLight>> pLights = app->getScenes().at(activeScene)->getPLights();
	std::shared_ptr<PointLight> light = pLights.at(0);
	float brightness = light->getBrightness();
	glm::vec3 lightPosition = light->getPosition();

	// get objects of the scene
	std::vector<std::shared_ptr<Object>> objs = app->getScenes().at(activeScene)->getObjects();

	// get all meshes of the scene
	std::vector<std::shared_ptr<Mesh>> meshes;
	for(int i{0}; i < objs.size(); ++i)
	{
		std::vector<std::shared_ptr<Mesh>> current = objs.at(i)->getMeshes();
		meshes.insert(meshes.end(), current.begin(), current.end());
	}

	// width and height of render view
	int width = client->getWidth();
	int height = client->getHeight();

	glm::vec3 primRay;

	glm::vec3 pHit;
	glm::vec3 nHit;
	glm::vec2 texCoords;
	
	glm::vec3 fragPos;
	glm::vec3 fragNormal;
	glm::vec2 fragTexCoords;

	struct PixelShade pixShade;
	float shadow;
	int rgb[3];

	float fragDist{std::numeric_limits<float>::max()};
	float prevFragDist{std::numeric_limits<float>::max()};
	std::shared_ptr<Mesh> m;
	bool inter{false};

	for(int r{id}; r < height; r += nbThr)
	{
		for(int c{0}; c < width; ++c)
		{
			RT::computePrimaryRay(c, r, width, height, cam, primRay);
			for(int i{0}; i < meshes.size(); ++i)
			{
				if(RT::intersect(meshes.at(i), camPos, primRay, pHit, nHit, texCoords))
				{
					fragDist = RT::distance(camPos, pHit);
					if(fragDist < prevFragDist)
					{
						prevFragDist = fragDist;
						m = meshes.at(i);
						inter = true;

						fragPos = pHit;
						fragNormal = nHit;
						fragTexCoords = texCoords;
					}
				}
			}

			if(inter)
			{
				// get pixel shade data
				pixShade = RT::computePixelShade(m, fragPos, fragNormal, fragTexCoords, lightPosition, primRay);
				
				// compute shadow factor
				shadow = RT::computeShadowFactor(fragPos, fragNormal, light->getSamplePoints(), meshes);
				
				// shade
				rgb[0] = std::min(static_cast<int>(pixShade.color.x * (1.0f - shadow) * (pixShade.diffuse + pixShade.specular) * brightness * 255.0f), 255);
				rgb[1] = std::min(static_cast<int>(pixShade.color.y * (1.0f - shadow) * (pixShade.diffuse + pixShade.specular) * brightness * 255.0f), 255);
				rgb[2] = std::min(static_cast<int>(pixShade.color.z * (1.0f - shadow) * (pixShade.diffuse + pixShade.specular) * brightness * 255.0f), 255);

				// draw color
				int pixel = 3 * ((r - 1) * width + c);
				img[pixel] = rgb[0];
				img[pixel+1] = rgb[1];
				img[pixel+2] = rgb[2];
			}

			inter = false;
			fragDist = std::numeric_limits<float>::max();
			prevFragDist = std::numeric_limits<float>::max();
		}
	}

	return 1;
}

void RT::computePrimaryRay(int& x, int& y, const int& scrX, const int& scrY, std::shared_ptr<Camera> & cam, glm::vec3 & primRay)
{
	// aspect ratio
	float aspectRatio = static_cast<float>(scrX) / static_cast<float>(scrY);

	// map x coordinate to range [-1,1]
	float ndcX = ((static_cast<float>(x) / static_cast<float>(scrX)) - 0.5f) * 2.0f;

	// map y coordinate to range [-1,1]
	float ndcY = ((static_cast<float>(y) / static_cast<float>(scrY)) - 0.5f) * 2.0f;

	// compute factor of contribution constant (max value)
	float f = sin(cam->getFov() * 2.0f);

	// amount of right and up vector to get
	float rightAmount = ndcX * f * aspectRatio;
	float upAmount = ndcY * f;

	// result
	primRay = cam->getDirection() + rightAmount * cam->getRight() + upAmount * cam->getUp();
}

float RT::computeShadowFactor(const glm::vec3 & fragPos, const glm::vec3 & fragNormal, std::vector<glm::vec3> & lightSamples, std::vector<std::shared_ptr<Mesh>> & meshes)
{
	std::vector<glm::vec3> rayDirections;
	for(int i{0}; i < lightSamples.size(); ++i)
	{
		glm::vec3 sample = lightSamples.at(i);
		rayDirections.push_back(glm::normalize(sample - fragPos));
	}
	glm::vec3 offset = 0.001f * glm::normalize(fragNormal);
	glm::vec3 rayOrigin = fragPos + offset;

	// begin useless data
	glm::vec3 pHit;
	glm::vec3 nHit;
	glm::vec2 tCoords;
	// end useless data
	
	float shadow{0.0f};

	for(int j{0}; j < rayDirections.size(); ++j)
	{
		for(int i{0}; i < meshes.size(); ++i)
		{
			if(RT::intersect(meshes.at(i), rayOrigin, rayDirections.at(j), pHit, nHit, tCoords))
			{
				shadow += 1.0f;
				break;
			}
		}
	}
	
	return shadow / static_cast<float>(rayDirections.size());
}

float RT::computeDiffuse(const glm::vec3 & pHit, const glm::vec3 & nHit, const glm::vec3 & lightPosition)
{
	glm::vec3 lightDir = glm::normalize(lightPosition - pHit);
	return std::max(glm::dot(lightDir, glm::normalize(nHit)), 0.0f);
}

float RT::computeSpecular(const glm::vec3 & pHit, const glm::vec3 & nHit, const glm::vec3 & lightPosition, const glm::vec3 & primRay, const float & shininess)
{
	glm::vec3 lightDir = glm::normalize(lightPosition - pHit);
	glm::vec3 halfwayDir = glm::normalize(-primRay + lightDir);
	float spec = std::max(glm::dot(nHit, halfwayDir), 0.0f);
	spec = pow(spec, shininess);
	return spec;
}

struct PixelShade RT::computePixelShade(std::shared_ptr<Mesh> & m, glm::vec3 & fragPos, glm::vec3 & fragNormal, glm::vec2 & fragTexCoords, glm::vec3 & lPos, glm::vec3 & primRay)
{
	// get mesh material
	Material mat = m->getMaterial();
	glm::vec3 color = mat.color;
	float shininess = mat.shininess;
	float specFactor{1.0f};
	std::vector<Texture> textures = mat.textures;

	int channels;
	int sampleX;
	int sampleY;
	int sampleIndex;
	int sample[3];

	for(int i{0}; i < textures.size() && i < 2; ++i)
	{
		channels = textures.at(i).channels;
		sampleX = static_cast<int>(fragTexCoords.x * (textures.at(i).width-1));
		sampleY = static_cast<int>(fragTexCoords.y * (textures.at(i).height-1));
		sampleIndex = channels * ((sampleY - 1) * textures.at(i).width + sampleX);
		sample[0] = textures.at(i).img[sampleIndex];
		sample[1] = textures.at(i).img[sampleIndex+1];
		sample[2] = textures.at(i).img[sampleIndex+2];

		if(i == 0)
			color = glm::vec4(sample[0] / 255.0f, sample[1] / 255.0f, sample[2] / 255.0f, 1.0f);
		else if(i == 1)
			specFactor = glm::vec4(sample[0] / 255.0f, sample[1] / 255.0f, sample[2] / 255.0f, 1.0f).x;
	}

	// get diffuse component
	float diffuse = RT::computeDiffuse(fragPos, fragNormal, lPos);

	// get specular component
	float specular = RT::computeSpecular(fragPos, fragNormal, lPos, primRay, shininess);
	specular *= specFactor;

	// PixelShade
	return PixelShade(color, diffuse, specular);
}

bool RT::intersect(std::shared_ptr<Mesh> & mesh, glm::vec3 & rayOrigin, glm::vec3 & rayDir, glm::vec3 & pHit, glm::vec3 & nHit, glm::vec2 & texCoords)
{
	// intersection distance from camera's origin
	float minDistance{std::numeric_limits<float>::max()};
	float prevMinDistance{minDistance};
	
	// barycentric coordinates (u,v,w)
	float u{0.0f};
	float v{0.0f};
	float w{1.0f - u - v};

	// get vertices
	std::vector<Vertex> vertices = mesh->getVertices();

	// get indices
	std::vector<int> indices = mesh->getIndices();

	// intersection ?
	bool res{false};

	for(int i{0}; i < indices.size(); i += 3)
	{
		glm::vec3 v0 = vertices.at(indices.at(i)).position;
		glm::vec3 normV0 = vertices.at(indices.at(i)).normal;
		glm::vec2 texV0 = vertices.at(indices.at(i)).texCoords;
		
		glm::vec3 v1 = vertices.at(indices.at(i+1)).position;
		glm::vec3 normV1 = vertices.at(indices.at(i+1)).normal;
		glm::vec2 texV1 = vertices.at(indices.at(i+1)).texCoords;
		
		glm::vec3 v2 = vertices.at(indices.at(i+2)).position;
		glm::vec3 normV2 = vertices.at(indices.at(i+2)).normal;
		glm::vec2 texV2 = vertices.at(indices.at(i+2)).texCoords;

		if(RT::rayTriangleIntersect(rayOrigin, rayDir, v0, v1, v2, minDistance, u, v))
		{
			if(minDistance < prevMinDistance && minDistance > 0.0f)
			{
				prevMinDistance = minDistance;
				res = true;
				w = 1.0f - u - v;
				pHit = rayOrigin + minDistance * rayDir;
				nHit = u * normV1 + v * normV2 + w * normV0;
				texCoords = u * texV1 + v * texV2 + w * texV0;
			}
		}
	}

	return res;
}

bool RT::rayTriangleIntersect(const glm::vec3 & origin, const glm::vec3 & dir, glm::vec3 & v0, glm::vec3 & v1, glm::vec3 & v2, float & t, float & u, float & v)
{
	const float EPSILON = 0.000'000'1f;
	glm::vec3 v0v1 = v1 - v0;
	glm::vec3 v0v2 = v2 - v0;
	glm::vec3 pvec = glm::cross(dir, v0v2);
	float det = glm::dot(v0v1, pvec);

	// if ray is back facing, or is parallel to the triangle's plane, then return false
	if(det < EPSILON || fabs(det) < EPSILON)
		return false;

	float invDet = 1.0f / det;

	glm::vec3 tvec = origin - v0;
	u = glm::dot(tvec, pvec) * invDet;
	if(u < 0 || u > 1)
		return false;

	glm::vec3 qvec = glm::cross(tvec, v0v1);
	v = glm::dot(dir, qvec) * invDet;
	if(v < 0 || u + v > 1)
		return false;

	t = glm::dot(v0v2, qvec) * invDet;

	return true;
}

float RT::distance(const glm::vec3 & v1, const glm::vec3 & v2)
{
	glm::vec3 v = v2 - v1;
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
