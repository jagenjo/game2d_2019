#include "image.h"
#include <algorithm>    // std::max, min, clamp
#include "utils.h"

template <typename T> T clamp(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}

std::map<std::string, Image*> Image::s_loaded_images;

Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height*sizeof(Color)];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if (pixels)
	{
		delete pixels;
		pixels = NULL;
	}
}


void Image::drawImage(const Image& img, int x, int y)
{
	if (x > (int)width || y > (int)height || (x + (int)img.width) < 0 || (y + (int)img.height) < 0)
		return; //outside

	int startx = clamp(x, 0, (int)width);
	int starty = clamp(y, 0, (int)height);
	int endx = clamp(x + img.width, 0, (int)width);
	int endy = clamp(y + img.height, 0, (int)height);

	//iterate 
	for (int i = startx; i < endx; ++i)
		for (int j = starty; j < endy; ++j)
		{
			const Color &c = img.getPixelRef(i - x, j - y);
			if (c.a == 0)
				continue;
			int pos = j * width + i;
			if (c.a == 255)
				pixels[pos] = c;
			else
				pixels[pos] = blendColors( c, pixels[pos] );
		}
}

void Image::drawImage(const Image& img, int x, int y, int w, int h)
{
	if (x > (int)width || y > (int)height || (x + (int)w) < 0 || (y + (int)h) < 0)
		return; //outside

	int startx = clamp(x, 0, (int)width);
	int starty = clamp(y, 0, (int)height);
	int endx = clamp(x + w, 0, (int)width);
	int endy = clamp(y + h, 0, (int)height);
	float fx = w / (float)img.width;
	float fy = h / (float)img.height;

	//iterate 
	for (int i = startx; i < endx; ++i)
		for (int j = starty; j < endy; ++j)
		{
			int localx = (i - x) / fx;
			int localy = (j - y) / fy;
			const Color &c = img.getPixelSafe(localx, localy);
			if (c.a == 0)
				continue;
			int pos = j * width + i;
			if (c.a == 255)
				pixels[pos] = c;
			else
				pixels[pos] = blendColors(c, pixels[pos]);
		}
}


void Image::drawImage(const Image& img, int x, int y, int imgx, int imgy, int imgw, int imgh)
{
	if (x > (int)width || y > (int)height || (x + (int)imgw) < 0 || (y + (int)imgh) < 0)
		return; //outside

	imgx = clamp(imgx, 0, (int)img.width);
	imgy = clamp(imgy, 0, (int)img.height);
	imgw = clamp(imgw, 0, (int)img.width - imgx);
	imgh = clamp(imgh, 0, (int)img.height - imgy);
	int startx = clamp(x, 0, (int)width);
	int starty = clamp(y, 0, (int)height);
	int endx = clamp(x + imgw, 0, (int)width);
	int endy = clamp(y + imgh, 0, (int)height);

	//iterate 
	for (int i = startx; i < endx; ++i)
		for (int j = starty; j < endy; ++j)
		{
			const Color &c = img.getPixel(i - x + imgx, j - y + imgy);
			if (c.a == 0)
				continue;
			int pos = j * width + i;
			if (c.a == 255)
				pixels[pos] = c;
			else
				pixels[pos] = blendColors(c, pixels[pos]);
		}
}

void Image::drawImage(const Image& img, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) //draws only a part of the image
{
	if (dx > (int)width || dy > (int)height || (dx + (int)dw) < 0 || (dy + (int)dh) < 0)
		return; //outside

	sx = clamp(sx, 0, (int)img.width);
	sy = clamp(sy, 0, (int)img.height);
	sw = clamp(sw, 0, (int)img.width - sx);
	sh = clamp(sh, 0, (int)img.height - sy);

	float deltax = sw / (float)dw;
	float deltay = sh / (float)dh;

	//iterate 
	for (float i = 0; i < dw; i += 1)
		for (float j = 0; j < dh; j += 1)
		{
			int px = i*deltax + sx;
			int py = j*deltay + sy;
			if (px < 0 || py < 0 || px >= img.width || py >= img.height)
				continue;
			int px2 = i + dx;
			int py2 = j + dy;
			if (px2 < 0 || py2 < 0 || px2 >= width || py2 >= height)
				continue;
			const Color &c = img.getPixel(px,py);
			if (c.a == 0)
				continue;
			int pos = py2 * width + px2;
			if (c.a == 255)
				pixels[pos] = c;
			else
				pixels[pos] = blendColors(c, pixels[pos]);
		}
}



void Image::drawLine(int x0, int y0, int x1, int y1, const Color& c)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */
	for (;;) {  /* loop */
		if( x0 >= 0 && y0 >= 0 && x0 < (int)width && y0 < (int)height)
			blendPixel( x0, y0, c );
		if (x0 == x1 && y0 == y1)
			break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}

void Image::drawText(std::string text, int x, int y, const Image& bitmapfont, int font_w, int font_h, int first_char )
{
	int startx = x;
	for (int i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			y += font_h;
			x = startx;
			continue;
		}
		drawImage( bitmapfont, x, y, bitmapfont.getArea(text[i] - first_char, font_w, font_h));
		x += font_w;
	}
}

void Image::drawRectangle(int x, int y, int w, int h, const Color& c)
{
	if (c.a == 0)
		return;

	if (x >(int)width || y > (int)height || (x + (int)w) < 0 || (y + (int)h) < 0)
		return; //outside

	int startx = clamp(x, 0, (int)width);
	int starty = clamp(y, 0, (int)height);
	int endx = clamp(x + w, 0, (int)width);
	int endy = clamp(y + h, 0, (int)height);

	for (int i = startx; i < endx; ++i)
		for (int j = starty; j < endy; ++j)
		{
			int pos = j * width + i;
			if (c.a == 255)
				pixels[pos] = c;
			else
				pixels[pos] = blendColors(c, pixels[pos]);
		}
}

void Image::crop(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	memset(new_pixels, 0, width * height * sizeof(Color));
	if (pixels)
	{
		for (unsigned int i = 0; i < width; ++i)
			for (unsigned int j = 0; j < height; ++j)
				new_pixels[j * width + i] = getPixelSafe(i - x, j - y);
		delete pixels;
	}
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	Image img(width,height);
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;
	for (unsigned int i = 0; i < min_width; ++i)
		for (unsigned int j = 0; j < min_height; ++j)
			img.pixels[j * width + i] = getPixel(i - x, j - y);
	return img;
}


//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	if (this->width == width && this->height == height)
		return;
	crop(0, 0, width, height);
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	if (this->width == width && this->height == height)
		return;

	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

void Image::flipX()
{
	for(unsigned int x = 0; x < width * 0.5; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixel( width - x - 1, y, getPixel(x,y));
			setPixel( x, y, temp );
		}
}

void Image::flipY()
{
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixel( x, height - y - 1, getPixel(x,y) );
			setPixel( x, y, temp );
		}
}

//reduce color palette quantizing every channel
void Image::quantize(int levels) 
{
	if (levels < 1)
		return;
	unsigned int l = width * height;
	for (unsigned int i = 0; i < l; ++i)
	{
		Color& c = pixels[i];
		c.r = (round((c.r / 255.0) * levels) / levels) * 255;
		c.g = (round((c.g / 255.0) * levels) / levels) * 255;
		c.b = (round((c.b / 255.0) * levels) / levels) * 255;
	}
}

Area Image::getArea(int index, int w, int h) const
{
	int f = (index * w);
	return Area(f % width, floor(f / width) * h, w, h);
}


void Image::maskAlpha(const Color& alpha_color)
{
	unsigned int l = width * height;
	for (unsigned int i = 0; i < l; ++i)
	{
		Color& c = pixels[i];
		if (c.r == alpha_color.r && c.g == alpha_color.g && c.b == alpha_color.b)
			c.a = 0;
	}
}

//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE * file = fopen(filename, "rb");
	if (file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << filename << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;

	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];

	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "TGA file seems to have errors or it is compressed, only uncompressed TGAs supported" << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}

	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;

	tgainfo->data = new unsigned char[imageSize];

	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;

		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	//save info in image
	if (pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];
	bool flip = true;

	//convert tga pixel format
	for (unsigned int y = 0; y < height; ++y)
		for (unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			unsigned int alpha = bytesPerPixel == 4 ? tgainfo->data[pos + 3] : 255;
			this->setPixel(x, flip ? height - y - 1 : y, Color(tgainfo->data[pos + 2], tgainfo->data[pos + 1], tgainfo->data[pos], alpha));
		}

	delete tgainfo->data;
	delete tgainfo;
	std::cout << " + Image loaded: " << filename << std::endl;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	FILE *file = fopen(filename, "wb");
	if (file == NULL)
	{
		fclose(file);
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	//tgainfo->width = header[1] * 256 + header[0];
	//tgainfo->height = header[3] * 256 + header[2];

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	//convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height * 3];
	for (unsigned int y = 0; y < height; ++y)
		for (unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height - y - 1)*width + x];
			unsigned int pos = (y*width + x) * 3;
			bytes[pos + 2] = c.r;
			bytes[pos + 1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height * 3, file);
	fclose(file);
	return true;
}

void Image::setName(std::string name)
{
	this->name = name;
	s_loaded_images[name] = this;
}

Image* Image::Get(std::string name)
{
	auto it = s_loaded_images.find(name);
	if (it != s_loaded_images.end())
		return it->second;
	Image* img = new Image();
	img->loadTGA(name.c_str());
	img->setName(name);
	return img;
}
