#include "Image.h"

class ImageIO
{
	public:
		ImageIO();
		~ImageIO();
		
		int getWidth() const;
		int getHeight() const;

		void draw();
		void loadFile(const std::string filename);
		void saveImage(const std::string filename);
		void setPixmap(int w, int h, unsigned char *fromPixmap);

	private:
		void buildMap();
		void setupRGBpixmap(int w, int h);
		void setupGRAYpixmap(int w, int h);
		int  getNchannels(const std::string filename);

		int width, height; // image size
		unsigned char *pixmap;
		unsigned char *RGBPixmap;
		unsigned char *GRAYPixmap;	
};