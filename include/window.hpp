#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <iostream>
#include <string>
#include <array>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <bitset>
#include <omp.h>
#include <cmath>
#include <memory>
#include "color.hpp"

struct WindowEvent
{
	SDL_Event e;
	const Uint8* keyboardState; // snapshot of the keyboard state
	Uint32 mouseButtonBitMask;
};

// about the bitset which register user inputs
// 0 => right mouse button
// 1 => middle mouse button
// 2 => left mouse button
// 3 => wheel scroll

class WindowManager
{
	public:

		/**
 		* \brief Creates a SDL window, with en OPENGL context, returns a pointer
 		* to the newly allocated window, or nullptr on failure.
 		*/
		WindowManager(const std::string& title);
		~WindowManager();
		int getWidth();
		int getHeight();
		SDL_Window* getWindowPtr();
		SDL_Window* getRenderViewPtr();
		SDL_Renderer* getRendererPtr();
		bool isMainWindowFocused();
		bool isRenderViewFocused();
		std::array<int, 3> & getMouseData();
		std::bitset<16> & getUserInputs();
		bool isAlive();
		bool getShowRenderView();
		void setShowRenderView(bool show);
		void checkEvents();
		void resetEvents();

	private:

		std::string title;
		int width;
		int height;
		bool alive;
		bool showRenderView;

		SDL_Window * window;
		Uint32 windowID;
		SDL_Window * renderView;
		SDL_Renderer * renderer;
		Uint32 renderViewID;
		SDL_GLContext glContext;

		bool mainWindowFocus;
		bool renderViewFocus;

		struct WindowEvent event;
		std::array<int, 3> mouseData; // 0 = xRel, 1 = yRel, 2 = mouse wheel direction
		std::bitset<16> userInputs;
};

#endif
