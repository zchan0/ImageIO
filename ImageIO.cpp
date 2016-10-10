#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <OpenImageIO/imageio.h>
#include <iostream>

#include "ImageIO.h"

OIIO_NAMESPACE_USING

static std::map<std::string, int>format_nchannels_map;

ImageIO::ImageIO()
{
	image = Image();
	buildMap();
}

ImageIO::~ImageIO()
{}

void ImageIO::loadImage(const std::string filename)
{
	ImageInput *inFile = ImageInput::open(filename);
	if (!inFile) {
		std::cerr << "ERROR: could not open " << filename << ", error = " << geterror() << std::endl; 
    	return;
	}

	const ImageSpec &spec = inFile -> spec();
	int w = spec.width;
	int h = spec.height;
	int nchannels = spec.nchannels;
	
	image = Image(w, h, nchannels);
	if (image.empty()) {
		std::cerr << "ERROR: could not init image, check width " << w << " height " << h << " nchannels " << nchannels << std::endl;
		return;
	}

	bool succeed;
	switch(nchannels) {
		case RGB:
			setupRGBpixmap(w, h);
			succeed = inFile -> read_image(TypeDesc::UINT8, RGBPixmap); 
			image.toRGBA(RGBPixmap, RGB, w, h);
			break;
		case GRAY:
			setupGRAYpixmap(w, h);
			succeed = inFile -> read_image(TypeDesc::UINT8, GRAYPixmap);
			image.toRGBA(GRAYPixmap, GRAY, w, h);
			break;
		case RGBA:
		default:
			succeed = inFile -> read_image(TypeDesc::UINT8, image.pixmap);
			break;
	}

	if (!succeed) {
		std::cerr << "ERROR: could not read image file " << filename << ", error = " << geterror() << std::endl;
    	delete inFile;
    	return;
	}

	image.flip();

	if (!inFile -> close()) {
		std::cerr << "ERROR: could not close " << filename << ", error = " << geterror() << std::endl;
    	delete inFile;
    	return;	
	}

	delete inFile;
}

void ImageIO::saveImage(const std::string filename)
{
	ImageOutput *outFile = ImageOutput::create(filename);
  	if (!outFile) {
    	std::cerr << "ERROR: could not create output image for " << filename << ", error = " << geterror() << std::endl;
    	return;
  	}	

  	// get image nchannels from input file name
	int nchannels = getNchannels(filename);
	if (nchannels == -1) { 
	  std::cout << "ERROR: not support for " << filename << std::endl;
	  return;
	}

	// get current window size 
  	int w = glutGet(GLUT_WINDOW_WIDTH);
  	int h = glutGet(GLUT_WINDOW_HEIGHT);
  	Image image = Image(w, h, nchannels);

  	if (image.empty()) {
		std::cerr << "ERROR: could not init image, check width " << w << " height " << h << " nchannels " << nchannels << std::endl;
		return;
	}

  	ImageSpec spec(w, h, nchannels, TypeDesc::UINT8);
  	// always use GL_RGBA here
  	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &image.pixmap[0]);

  	if (!outFile -> open(filename, spec)) {
  		std::cerr << "ERROR: could not open " << filename << ", error = " << geterror() << std::endl;
  		delete outFile;
  		return;
  	}

  	image.flip();

  	bool succeed;
  	switch(nchannels) {
  		case RGB:
  			setupRGBpixmap(w, h);
  			image.toRGB(RGBPixmap, w, h);
  			succeed = outFile -> write_image(TypeDesc::UINT8, RGBPixmap);
  			break;
  		case GRAY:
  			setupGRAYpixmap(w, h);
  			image.toGRAY(GRAYPixmap, w, h);
  			succeed = outFile -> write_image(TypeDesc::UINT8, GRAYPixmap);
  			break;
  		case RGBA:
  		default:
  			succeed = outFile -> write_image(TypeDesc::UINT8, image.pixmap);
  			break;
  	}

  	if (!succeed) {
  		std::cerr << "ERROR: could not write image to " << filename << ", error = " << geterror() << std::endl;
  		delete outFile;
  		return;
  	}

  	if (!outFile -> close()) {
  		std::cerr << "ERROR: could not close " << filename << ", error = " << geterror() << std::endl;
    	delete outFile;
    	return;	
  	}

  	std::cout << "Save " << filename << " succeed!" << std::endl;

  	delete outFile;
}

void ImageIO::drawImage() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  if(image.pixmap != NULL) {
    int w = glutGet(GLUT_WINDOW_WIDTH); 
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    int iw = image.getWidth();
    int ih = image.getHeight();

    // calculate portion of image size to current window size
    // factorX/Y < 1 means current window size need resize image
    // use the smallest factor to fit current window
    double factorX = (double)w / iw;
    double factorY = (double)h / ih;
    double factor = std::min(1.0, std::min(factorX, factorY));

    // calculate center postion
    // if image width or height small than window's, corresponding raster x or y should be 0
    // if window width or height bigger than image's, corresponding raster x or y should be in center 
    int centerX = factorX < 1 ? 0 : (w - iw) / 2;
    int centerY = factorY < 1 ? 0 : (h - ih) / 2;

    glPixelZoom(factor, factor);
    glRasterPos2i(centerX, centerY);
   
    //glDrawPixels writes a block of pixels to the frame buffer.
    glDrawPixels(iw, ih, GL_RGBA, GL_UNSIGNED_BYTE, image.pixmap);
  }

  // swap the back and front buffers so can see what just drew
  glutSwapBuffers();
}

int ImageIO::getNchannels(const std::string filename)
{
	std::string fileFormat;
  	const int pos = filename.find_last_of('.');
  	// extract image format from filename
  	fileFormat = filename.substr(pos + 1); 
  
	return format_nchannels_map[fileFormat] ? : -1;
}

void ImageIO::buildMap()
{
	format_nchannels_map["ppm"] = RGB;
  	format_nchannels_map["png"] = format_nchannels_map["jpg"] = format_nchannels_map["tif"] = RGBA;
}

void ImageIO::setupRGBpixmap(int w, int h)
{
	if (RGBPixmap != NULL) return;
	RGBPixmap = new unsigned char[RGB * w * h];
}

void ImageIO::setupGRAYpixmap(int w, int h)
{
	if (GRAYPixmap != NULL) return;
	GRAYPixmap = new unsigned char[GRAY * w * h];
}