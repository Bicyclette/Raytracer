#include "window.hpp"

WindowManager::WindowManager(const std::string& title)
{
	alive = true;
	showRenderView = false;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << SDL_GetError() << std::endl;
		std::exit(-1);
	}

	// Get screen size and create main window
	SDL_Rect r;
	if(SDL_GetDisplayBounds(0, &r) != 0)
	{
		std::cerr << SDL_GetError() << std::endl;
		std::exit(-1);
	}

	width = ceil(r.w / 1.25f);
	height = ceil(r.h / 1.25f);

	window = nullptr;

	// OPENGL VERSION
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

	// DOUBLE BUFFER
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// MULTISAMPLING
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	window = SDL_CreateWindow(
				title.c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				width,
				height,
				SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
				);

	if(window == nullptr)
	{
		std::cerr << SDL_GetError() << std::endl;
		SDL_Quit();
		std::exit(-1);
	}

	windowID = SDL_GetWindowID(window);

	// create render view window
	renderView = SDL_CreateWindow(
				"render view",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				width,
				height,
				SDL_WINDOW_HIDDEN
				);

	if(renderView == nullptr)
	{
		std::cerr << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		std::exit(-1);
	}

	renderViewID = SDL_GetWindowID(renderView);

	// create renderer
	renderer = SDL_CreateRenderer(renderView, -1, SDL_RENDERER_ACCELERATED);

	// set focus on main window
	SDL_SetWindowInputFocus(window);

	// focus data
	mainWindowFocus = true;
	renderViewFocus = false;

	// OpenGL stuff
	glContext = SDL_GL_CreateContext(window);

	// Glew init
	glewExperimental = true;
	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		std::cerr << glewGetErrorString(err) << std::endl;
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(window);
		SDL_Quit();
		std::exit(-1);
	}

	// Setting OpenGL states
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(LIGHT_GREY[0], LIGHT_GREY[1], LIGHT_GREY[2], LIGHT_GREY[3]);
	SDL_GL_SetSwapInterval(1);
}

WindowManager::~WindowManager()
{
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(renderView);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int WindowManager::getWidth()
{
	return width;
}

int WindowManager::getHeight()
{
	return height;
}

SDL_Window* WindowManager::getWindowPtr()
{
	return window;
}

SDL_Window* WindowManager::getRenderViewPtr()
{
	return renderView;
}

SDL_Renderer* WindowManager::getRendererPtr()
{
	return renderer;
}

bool WindowManager::isMainWindowFocused()
{
	return mainWindowFocus;
}

bool WindowManager::isRenderViewFocused()
{
	return renderViewFocus;
}

std::array<int, 3> & WindowManager::getMouseData()
{
	return mouseData;
}

std::bitset<16> & WindowManager::getUserInputs()
{
	return userInputs;
}

bool WindowManager::isAlive()
{
	return alive;
}

bool WindowManager::getShowRenderView()
{
	return showRenderView;
}

void WindowManager::setShowRenderView(bool show)
{
	showRenderView = show;
}

void WindowManager::checkEvents()
{
	event.keyboardState = SDL_GetKeyboardState(nullptr);
	event.mouseButtonBitMask = SDL_GetRelativeMouseState(&mouseData[0], &mouseData[1]);

	while(SDL_PollEvent(&event.e))
	{
		if(event.e.type == SDL_WINDOWEVENT)
		{
			if(event.e.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				Uint32 id = event.e.window.windowID;
				if(id == windowID)
					alive = false;
				if(id == renderViewID)
					showRenderView = false;
			}
			if(event.e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				userInputs.set(5);
				width = event.e.window.data1;
				height = event.e.window.data2;
				glViewport(0, 0, width, height);
				SDL_SetWindowSize(renderView, width, height);
			}
			if(event.e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				Uint32 id = event.e.window.windowID;
				if(id == windowID)
					mainWindowFocus = true;
				else if(id == renderViewID)
					renderViewFocus = true;
			}
			if(event.e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				Uint32 id = event.e.window.windowID;
				if(id == windowID)
					mainWindowFocus = false;
				else if(id == renderViewID)
					renderViewFocus = false;
			}
		}

		if(event.e.type == SDL_MOUSEWHEEL)
		{
			userInputs.set(3);
			mouseData[2] = event.e.wheel.y;
		}
	}

	if(SDL_BUTTON(event.mouseButtonBitMask) == SDL_BUTTON_LEFT)
	{
		userInputs.set(2);
	}
	
	if(SDL_BUTTON(event.mouseButtonBitMask) == SDL_BUTTON_MIDDLE)
	{
		userInputs.set(1);
	}

	if(event.keyboardState[SDL_SCANCODE_LSHIFT])
	{
		userInputs.set(4);
	}

	if(event.keyboardState[SDL_SCANCODE_LCTRL])
	{
		userInputs.set(6);
	}
	
	if(event.keyboardState[SDL_SCANCODE_R])
	{
		userInputs.set(7);
		showRenderView = true;
	}
	
	if(event.keyboardState[SDL_SCANCODE_S])
	{
		userInputs.set(8);
	}
}

void WindowManager::resetEvents()
{
	userInputs.reset();
}
