/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This file has all the includes so the app works in different systems.
	It is a little bit low level so do not worry about the code.
*/

#ifndef INCLUDES_H
#define INCLUDES_H

//under windows we need this file to make opengl work
#ifdef WIN32 
	#include <windows.h>
#endif

//remove warnings about unsafe functions
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

//SDL
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


//GLUT
#ifdef WIN32
	#include <GL/glut.h>
#endif

#ifdef __APPLE__
	#include <GLUT/glut.h>
#endif

#include <iostream>
#include <cmath>
#include <string>
#include <iostream>

//OpenGL
//#include <GL/glu.h> //including GLUT we include everything (opengl, glu and glut)

//used to access opengl extensions
//void* getGLProcAddress(const char*);
#define REGISTER_GLEXT(RET, FUNCNAME, ...) typedef RET (APIENTRY * FUNCNAME ## _func)(__VA_ARGS__); FUNCNAME ## _func FUNCNAME = NULL; 
#define IMPORT_GLEXT(FUNCNAME) FUNCNAME = (FUNCNAME ## _func) SDL_GL_GetProcAddress(#FUNCNAME); if (FUNCNAME == NULL) { std::cout << "ERROR: This Graphics card doesnt support " << #FUNCNAME << std::endl; }

#endif
