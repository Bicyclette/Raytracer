#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include "window.hpp"
#include "application.hpp"
#include "framebuffer.hpp"

void render(std::shared_ptr<WindowManager> & client, std::shared_ptr<Application> raytracer)
{
	// choose scene to render (0: minecraft, 1: angel)
	raytracer->setActiveScene(1);

	// delta
	double currentFrame = 0.0f;
	double lastFrame = 0.0;
	
	while(client->isAlive())
	{
		client->checkEvents();
		currentFrame = omp_get_wtime();
		raytracer->updateSceneActiveCameraView(raytracer->getActiveScene(), client->getUserInputs(), client->getMouseData(), static_cast<float>(currentFrame - lastFrame));

		if(client->getUserInputs().test(5))
		{
			raytracer->resizeScreen(client->getWidth(), client->getHeight());
		}

		if(client->getUserInputs().test(6) && client->getUserInputs().test(7))
		{
			raytracer->renderScene(client, true);
		}
		
		// draw scene
		raytracer->drawScene(raytracer->getActiveScene(), client->getWidth(), client->getHeight(), DRAWING_MODE::SOLID, true);
		
		client->resetEvents();
		SDL_GL_SwapWindow(client->getWindowPtr());
		lastFrame = currentFrame;
	}
}

int main(int argc, char* argv[])
{
	std::shared_ptr<WindowManager> client{std::make_unique<WindowManager>("Raytracer")};
	std::shared_ptr<Application> raytracer{std::make_unique<Application>(client->getWidth(), client->getHeight())};
	render(client, raytracer);

	return 0;
}
