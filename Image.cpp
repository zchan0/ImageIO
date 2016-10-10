#include <iostream>

#include "Image.h"

Image::Image()
{
	pixmap = NULL;
	filename = channelname = "";
	width  = height = nchannels = 0;
}

Image::~Image()
{}

Image::Image(const Image& img)
{
	copy(img);
}

Image::Image(int w, int h, int nch): width(w), height(h), nchannels(nch)
{
	buildPixmap();
	filename = channelname = "";
}

Image& Image::operator=(const Image& rhs) 
{
	if (this == &rhs) return *this;
	copy(rhs);
	return *this;
}

void Image::copy(const Image& img)
{
	width  = img.width;
	height = img.height;
	filename  = img.filename;
	nchannels = img.nchannels; 	
	channelname = img.channelname;

	buildPixmap();

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			for (int channel = 0; channel < nchannels; ++channel) {
				switch(channel) {
					case R:
						pixmap[i][j].r = img.colorValue(img.pixmap[i][j], channel);	break;	
					case G:
						pixmap[i][j].g = img.colorValue(img.pixmap[i][j], channel);	break;	
					case B:
						pixmap[i][j].b = img.colorValue(img.pixmap[i][j], channel);	break;	
					case A:
						pixmap[i][j].a = img.colorValue(img.pixmap[i][j], channel);	break;	
					default:
						break;
				}
			}
		}
	}
}

/**
 * Use width, height to set up a pixmap
 * Always set pixmap to be black, opaque, use as many channel as needed 
 */
void Image::buildPixmap()
{
	pixmap = new Pixel*[height];	
	for (int i = 0; i < height; ++i) {
		pixmap[i] = new Pixel[width];
		for (int j = 0; j < width; ++j) {
			pixmap[i][j].r = 0;
			pixmap[i][j].g = 0;
			pixmap[i][j].b = 0;
			pixmap[i][j].a = 255;	
		}
	}
}

/**
 * OpenGL treats lower-left as origin, 
 * To display on screen or export image, needs to flip to upper-left first
 */
void Image::flip()
{
	for (int i = height - 1; i < (height + 1) / 2 - 1; --i) {
		for (int j = 0; j < width; ++j) {
			Pixel temp;
			temp = pixmap[i][j];
			pixmap[i][j] = pixmap[height - 1 - i][j];
			pixmap[height - 1 - i][j] = temp;	
		}
	}
}

void Image::tofloat()
{
	for (int channel = 0; channel < RGBA; ++channel) {
		floatPixmap[channel] = new float*[height];
		if (floatPixmap[channel] != NULL) {
			floatPixmap[channel][0] = new float[width * height];
		}
		if (floatPixmap[channel] == NULL || floatPixmap[channel][0] == NULL) {
			std::cerr << "ERROR: could not allocate memory for floatPixmap" << std::endl;
			exit(-1);
		}
		for (int i = 0; i < height; ++i) {
			floatPixmap[channel][i] = floatPixmap[channel][i - 1] + width;
		}
	}

	for (int channel = 0; channel < RGBA; ++channel) 
		for (int i = 0; i < height; ++i) 
			for (int j = 0; j < width; ++j) 
				floatPixmap[channel][i][j] = colorValue(pixmap[i][j], channel) / 255.0;
}

/**
 * Convert scaled(0 ~ 1) pixmap to range 0 ~ 255
 */
void Image::topixmap()
{
	float val = 0.0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			val =  pixmap[i][j].r * 255.0;
			pixmap[i][j].r = (val < 0.0 ? 0 : (val > 255.0 ? 255 : val));
			val =  pixmap[i][j].g * 255.0;
			pixmap[i][j].g = (val < 0.0 ? 0 : (val > 255.0 ? 255 : val));
			val =  pixmap[i][j].b * 255.0;
			pixmap[i][j].b = (val < 0.0 ? 0 : (val > 255.0 ? 255 : val));
			val =  pixmap[i][j].a * 255.0;
			pixmap[i][j].a = (val < 0.0 ? 0 : (val > 255.0 ? 255 : val));
		}
	}
}

void Image::toRGB(unsigned char *RGBPixmap, int w, int h)
{
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			RGBPixmap[(i * w + j) * RGB + R] = colorValue(pixmap[i][j], R);
			RGBPixmap[(i * w + j) * RGB + G] = colorValue(pixmap[i][j], G);
			RGBPixmap[(i * w + j) * RGB + B] = colorValue(pixmap[i][j], B);
		}
	}
}

void Image::toGRAY(unsigned char *GRAYPixmap, int w, int h)
{
	for (int i = 0; i < h; ++i) 
		for (int j = 0; j < w; ++j) 
			GRAYPixmap[i * w + j] = colorValue(pixmap[i][j], R);
}

void Image::toRGBA(unsigned char *fromPixmap, int nchannels, int w, int h)
{
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			for (int channel = 0; channel < RGBA; ++channel) {
				// setColorValue(pixmap[i][j], fromPixmap[(i * w + j) * GRAY], channel);
				setColorValue(pixmap[i][j], fromPixmap[(i * w + j) * RGB + channel], channel);
			}
		}
	}
}

int Image::getWidth() const
{
	return width;
}

int Image::getHeight() const
{
	return height;
}

int Image::getNchannels() const
{
	return nchannels;
}

std::string Image::getFilename() const
{
	return filename;
}

void Image::setFilename(const std::string n)
{
	filename = n;
}

std::string Image::getChannelname() const
{
	return channelname;
}

unsigned char Image::colorValue(const Pixel pixel, const int channel) const
{
	switch(channel) {
		case 0: return pixel.r; break;
		case 1: return pixel.g; break;
		case 2: return pixel.b; break;
		case 3: return pixel.a; break;
		default:
			return 0; break;
	}
}

void Image::setColorValue(Pixel &pixel, unsigned char val, int channel)
{
	switch(channel) {
		case 0: pixel.r = val; break;
		case 1: pixel.g = val; break;
		case 2: pixel.b = val; break;
		case 3: pixel.a = val; break;
		default: break;
	}
}

bool Image::empty() const
{
	return pixmap == NULL;
}

void Image::print() const
{
	if (pixmap == NULL) {
		std::cout << "ERROR: image is empty" << std::endl;
		return;
	}

	std::cout << "pixmap" << std::endl;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			for (int channel = 0; channel < nchannels; ++channel) {
				std::cout << (int)colorValue(pixmap[i][j], channel) << " ";
			}
			std::cout << std::endl;
		}
	}

	std::cout << "width " << width << " height " << height << " nchannels " << nchannels << std::endl;
	
	if (!filename.empty())
		std::cout << "filename " << filename << std::endl;
	if (!channelname.empty()) 
		std::cout << "channelname" << channelname << std::endl;
}
