#include <string>

typedef struct
{
	unsigned char r, g, b, a;
} Pixel;

typedef enum 
{
	GRAY = 1,
	RGB	 = 3,
	RGBA = 4
} NChannels;

typedef enum 
{
	R = 0, 
	G = 1,
	B = 2,
	A = 3
} Channel;

class Image
{
	public:
		Image();
		Image(const Image& img);
		Image(int w, int h, int nch);
		Image& operator=(const Image& rhs);
		~Image();

		void flip();
		void tofloat();
		void topixmap();
		void print() const;
		void setFilename(const std::string n);
		void setColorValue(Pixel &pixel, unsigned char val, int channel);
		void toRGB( unsigned char *RGBPixmap,  int w, int h);
		void toGRAY(unsigned char *GRAYPixmap, int w, int h);
		void toRGBA(unsigned char *fromPixmap, int nchannels, int w, int h);

		int getWidth() const;
		int getHeight() const;
		int getNchannels() const;
		std::string getFilename() const;
		std::string getChannelname() const;
		unsigned char colorValue(const Pixel pixel, const int channel) const;

		bool empty() const;

		/** Always RGBA */
		Pixel **pixmap;

		/** Scaled pixmap to 0 ~ 1 */
		float **floatPixmap[RGBA]; 

	private:
		void buildPixmap();
		void copy(const Image& img);

		/** Information about image */
		std::string filename;
		std::string channelname;
		
		int width, height, nchannels;
};