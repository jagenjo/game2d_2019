/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com

	MAIN:
	 + This file creates the window and the game instance. 
	 + It also contains the mainloop
	 + This is the lowest level, here we access the system to create the opengl Context
	 + It takes all the events from SDL and redirect them to the game
*/

#include "includes.h"

#include "framework.h"
#include "utils.h"
#include "input.h"
#include "game.h"

#include <iostream> //to output
#include <fstream>
#include <string>

long last_time = 0; //this is used to calcule the elapsed time between frames

Game* game = NULL;

// *********************************
//create a window using SDL
SDL_Window* createWindow(const char* caption, int width, int height, bool fullscreen = false)
{
    int multisample = 4;
    bool retina = false; //change this to use a retina display

	//prepare SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	//set attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); //or 24
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	//antialiasing (disable this lines if it goes too slow)
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample ); //increase to have smoother polygons

	// Initialize the joystick subsystem
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	//create the window
	SDL_Window *window = SDL_CreateWindow(caption, 100, 100, width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|
                                          (retina ? SDL_WINDOW_ALLOW_HIGHDPI:0) |
                                          (fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0) );
	if(!window)
	{
		fprintf(stderr, "Window creation error: %s\n", SDL_GetError());
		exit(-1);
	}
  
	// Create an OpenGL context associated with the window.
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	//in case of exit, call SDL_Quit()
	atexit(SDL_Quit);

	//get events from the queue of unprocessed events
	SDL_PumpEvents(); //without this line asserts could fail on windows

	//launch glew to extract the opengl extensions functions from the DLL
	#ifdef USE_GLEW
		glewInit();
	#endif

	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);
	std::cout << " * Window size: " << window_width << " x " << window_height << std::endl;
	std::cout << " * Path: " << getPath() << std::endl;
	std::cout << std::endl;

	//init input
	Input::init(window);

	return window;
}


//The application main loop
void mainLoop()
{
	SDL_Event sdlEvent;

	long start_time = SDL_GetTicks();
	long now = start_time;
	long frames_this_second = 0;

	while (!game->must_exit)
	{
		Input::update();

		//render frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		game->render();
		SDL_GL_SwapWindow(game->window);

		//update events
		while(SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_QUIT: return; break; //EVENT for when the user clicks the [x] in the corner
			case SDL_MOUSEBUTTONDOWN: //EXAMPLE OF sync mouse input
				Input::mouse_state |= SDL_BUTTON(sdlEvent.button.button);
				game->onMouseButtonDown(sdlEvent.button);
				break;
			case SDL_MOUSEMOTION:
				game->onMouseMove(sdlEvent.motion);
				break;
			case SDL_MOUSEBUTTONUP:
				Input::mouse_state &= ~SDL_BUTTON(sdlEvent.button.button);
				game->onMouseButtonUp(sdlEvent.button);
				break;
			case SDL_MOUSEWHEEL:
				game->onMouseWheel(sdlEvent.wheel);
				break;
			case SDL_KEYDOWN:
				game->onKeyDown(sdlEvent.key);
				break;
			case SDL_KEYUP:
				game->onKeyUp(sdlEvent.key);
				break;
			case SDL_JOYBUTTONDOWN:
				game->onGamepadButtonDown(sdlEvent.jbutton);
				break;
			case SDL_JOYBUTTONUP:
				game->onGamepadButtonUp(sdlEvent.jbutton);
				break;
			case SDL_TEXTINPUT:
				// you can read the ASCII character from sdlEvent.text.text 
				break;
			case SDL_WINDOWEVENT:
				switch (sdlEvent.window.event) {
				case SDL_WINDOWEVENT_RESIZED: //resize opengl context
					game->onResize(sdlEvent.window.data1, sdlEvent.window.data2);
					break;
				}
			}
		}

        
		//compute delta time
		long last_time = now;
		now = SDL_GetTicks();
		double elapsed_time = (now - last_time) * 0.001; //0.001 converts from milliseconds to seconds
		double last_time_seconds = game->time;
        game->time = float(now * 0.001);
		game->elapsed_time = elapsed_time;
		game->frame++;
		frames_this_second++;
		if (int(last_time_seconds *2) != int(game->time*2)) //next half second
		{
			game->fps = frames_this_second*2;
			frames_this_second = 0;
		}

		//update game logic
		game->update(elapsed_time); 

		//save old keyboard state
		memcpy((void*)&Input::prev_keystate, Input::keystate, SDL_NUM_SCANCODES);

		//check errors in opengl only when working in debug
		#ifdef _DEBUG
			checkGLErrors();
		#endif
	}

	return;
}

int main(int argc, char **argv)
{
	std::cout << "Initiating game..." << std::endl;

	//create the game window 
	SDL_Window* window = createWindow("TJE Game2D", 512, 512 );
	if (!window)
		return 0;

	//launch the game (game is a global variable)
	game = new Game(512, 512, window);

	//main loop, application gets inside here till user closes it
	mainLoop();

	return 0;
}
