#include "Image.h"

class ImageIO
{
	public:
		ImageIO();
		~ImageIO();
		
		void loadImage(const std::string filename);
		void saveImage(const std::string filename);
		void drawImage();
	
		/** image load in */
		Image image;

	private:
		void buildMap();
		void setupRGBpixmap(int w, int h);
		void setupGRAYpixmap(int w, int h);
		int  getNchannels(const std::string filename);

		/** handle nchannel != 4 */
		unsigned char *RGBPixmap;
		unsigned char *GRAYPixmap;	
};