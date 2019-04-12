/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This contains several functions that can be useful when programming your game.
*/
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <iostream>   
#include <vector>

#include "includes.h"
#include "framework.h"

class Shader;
class Texture;

//General functions **************
long getTime(); //returns time since computer started (in milliseconds)
long getPrecisionTime();
std::string toString(float v);

//generic rendering fuctions
void drawGrid();
int drawText(float x, float y, std::string text, Vector4 c, float scale = 1); //returns the width in pixels
void drawPoints(std::vector<Vector3> points, float size, Vector4 color);
void drawLines(std::vector<Vector3> points, Vector4 color, bool strip = false);
void drawLines2D( const std::vector<Vector3>& points, Vector4 color, bool strip = false);
void drawRect(float x, float y, float w, float h, Vector4 color, float angle = 0, bool fill = false);
void drawRect2D(float x, float y, float w, float h, Vector4 color, Texture* tex = NULL, bool wireframe = false, Shader* shader = NULL);

//check opengl errors
bool checkGLErrors();

//files
std::string getPath();
bool readFile(const std::string& filename, std::string& content);

//returns the size of the dekstop (not the window, the full desktop)
Vector2 getDesktopSize( int display_index = 0 );

std::vector<std::string> tokenize(const std::string& source, const char* delimiters, bool process_strings = false);
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

std::string getGPUStats();


inline int inrange(int value, int range) { value = value % range; if (value < 0) value += range; return value; }


#endif
