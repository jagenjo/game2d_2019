/*** image.h  Javi Agenjo (javi.agenjo@gmail.com) 2013
	This file defines the class Image that allows to manipulate images.
	It defines all the need operators for Color and Image.
	It has a TGA loader and saver.
***/

#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include "framework.h"

//remove unsafe warnings
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)


//Class Image: to store a matrix of pixels
class Image
{
	//a general struct to store all the information about a TGA file
	typedef struct sTGAInfo 
	{
		unsigned int width;
		unsigned int height;
		unsigned int bpp; //bits per pixel
		unsigned char* data; //bytes with the pixel information
	} TGAInfo;

public:
	unsigned int width;
	unsigned int height;
	Color* pixels;
	std::string name;

	// CONSTRUCTORS 
	Image();
	Image(unsigned int width, unsigned int height);
	Image(const Image& c);
	Image& operator = (const Image& c); //assign operator

	//destructor
	~Image();

	//get the pixel at position x,y
	Color getPixel(unsigned int x, unsigned int y) const { return pixels[ y * width + x ]; }
	Color& getPixelRef(unsigned int x, unsigned int y)	{ return pixels[ y * width + x ]; }
	const Color& getPixelRef(unsigned int x, unsigned int y) const { return pixels[y * width + x]; }
	Color getPixelSafe(unsigned int x, unsigned int y) const {
		x = clamp((unsigned int)x, 0, width-1); 
		y = clamp((unsigned int)y, 0, height-1); 
		return pixels[ y * width + x ]; 
	}
	Color getPixelRepeat(int x, int y) const { x %= width; y %= height; if (x < 0) x = width + x; if (y < 0) y = height + y; return pixels[y * width + x]; }

	//set the pixel at position x,y with value C
	inline void setPixel(unsigned int x, unsigned int y, const Color& c) const { pixels[y * width + x] = c; }
	inline void setPixelSafe(int x, int y, const Color& c) const { if (x < 0 || y < 0 || x >= width || y >= height) return; setPixel(x, y, c); }
	inline void blendPixel(unsigned int x, unsigned int y, const Color& c) const { Color& d = pixels[y * width + x]; d = blendColors(c, d); } //using the alpha to blend colors

	void resize(unsigned int width, unsigned int height); //resizes the canvas but keeping the data in the corner
	void scale(unsigned int width, unsigned int height); //stretches the image to fit the new size (slow)
	void crop(unsigned int x, unsigned int y, unsigned int width, unsigned int height); //crops the image to a given area
	Image getArea(unsigned int x, unsigned int y, unsigned int width, unsigned int height); //creates an image given an area

	//draw
	void drawImage(const Image& img, int x, int y);
	void drawImage(const Image& img, int x, int y, int w, int h); //draws an image but with different dimensions
	void drawImage(const Image& img, int x, int y, int imgx, int imgy, int imgw, int imgh); //draws only a part of the image
	void drawImage(const Image& img, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh); //draws only a part of the image
	void drawImage(const Image& img, int x, int y, Area rect) { drawImage(img, x, y, rect.x, rect.y, rect.w, rect.h); }//draws only a part of the image
	void drawLine( int x0, int y0, int x1, int y1, const Color& c);
	void drawText( std::string text, int x, int y, const Image& bitmapfont, int font_w = 7, int font_h = 9, int first_char = 32);
	void drawRectangle(int x, int y, int w, int h, const Color& c);

	void maskAlpha(const Color& alpha_color); //every pixel with the given color will be set to transparent

	void flipY(); //flip the image top-down
	void flipX(); //flip the image left-right

	//fill the image with the color C
	void fill(const Color& c) { for(unsigned int pos = 0; pos < width*height; ++pos) pixels[pos] = c; }
	void fillBlend(const Color& c) { for (unsigned int pos = 0; pos < width*height; ++pos) pixels[pos] = blendColors(c, pixels[pos]); } //fill image blending according to c.a (alpha)
	void multiplyByColor(const Color& c) { for (unsigned int pos = 0; pos < width*height; ++pos) { pixels[pos] = pixels[pos] * c; }	} //modulates the image by a color

	void quantize(int levels); //reduce color palette quantizing every channel to a limited amount of intensities
	Area getArea( int index, int w, int h) const; //returns a frame rect given the frame index and the width and height of every frame

	//save or load images from the hard drive
	bool loadTGA(const char* filename);
	bool saveTGA(const char* filename);

	//manager to load several images
	static Image* Get( std::string name );
	static std::map<std::string, Image*> s_loaded_images;
	void setName(std::string name);
};

inline Image operator * (const Image& a, const Image& b) {
	Image c(a.width, a.height);
	for (int x = 0; x < c.width; ++x)
		for (int y = 0; y < c.height; ++y)
			c.setPixel(x, y, a.getPixelRef(x, y) * b.getPixelRef(x, y));
	return c;
}

#endif
