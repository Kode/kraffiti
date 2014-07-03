#include "Image.h"
//#include "stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace kake;

Image::Image(int x, int y, unsigned* data) : x(x), y(y), data(data) {

}

Image::Image(byte* data, int size) {
	//int comp;
	//this->data = (unsigned*)stbi_load_from_memory(data, size, &x, &y, &comp, 4);
}

//Image::Image(kmd::Path path) {
//	int comp;
//	data = (unsigned*)stbi_load(path.toString().c_str(), &x, &y, &comp, 4);
//}

int Image::width() {
	return x;
}

int Image::height() {
	return y;
}

unsigned Image::pixel(int x, int y) {
	return data[y * width() + x];
}

unsigned Image::abgr(int x, int y) {
	return (a(x, y) << 24) | (b(x, y) << 16) | (g(x, y) << 8) | r(x, y);
}

unsigned Image::rgba(int x, int y) {
	return pixel(x, y); //(r(x, y) << 24) | (g(x, y) << 16) | (b(x, y) << 8) | a(x, y);
}

unsigned Image::argb(int x, int y) {
	return (a(x, y) << 24) | (r(x, y) << 16) | (g(x, y) << 8) | b(x, y);
}

byte Image::a(int x, int y) {
	return (pixel(x, y) & 0xff000000) >> 24;
}

byte Image::b(int x, int y) {
	return (pixel(x, y) & 0xff0000) >> 16;
}

byte Image::g(int x, int y) {
	return (pixel(x, y) & 0xff00) >> 8;
}

byte Image::r(int x, int y) {
	return pixel(x, y) & 0xff;
}

void Image::replaceAlpha(unsigned rgba) {

}

void Image::save(ByteStream& stream) {
	/*for (int y = 0; y < this->y; ++y) {
		for (int x = 0; x < this->x; ++x) {
			int val = rgba(x, y);
			unsigned uval = *reinterpret_cast<unsigned*>(&val);
			stream.put((byte) (uval >> 24));
			stream.put((byte) (uval >> 16));
			stream.put((byte) (uval >> 8));
			stream.put((byte) uval);
		}
	}*/
	//int length;
	//byte* pngdata = stbi_write_png_to_mem((byte*)data, 0, this->x, this->y, 4, &length);
	//for (int i = 0; i < length; ++i) {
	//	stream.put(pngdata[i]);
	//}
}

//void Image::save(kmd::Path path) {
	/*std::ofstream stream(path.toString(), std::ios::binary);
	for (int y = 0; y < this->y; ++y) {
		for (int x = 0; x < this->x; ++x) {
			int val = rgba(x, y);
			unsigned uval = *reinterpret_cast<unsigned*>(&val);
			stream.put((byte) (uval >> 24));
			stream.put((byte) (uval >> 16));
			stream.put((byte) (uval >> 8));
			stream.put((byte) uval);
		}
	}*/
//	stbi_write_png(path.toString().c_str(), this->x, this->y, 4, data, 0);
//}

static int roundi(float value) {
	return (int)std::floor(value + 0.5f);
}

unsigned Image::sample(float x, float y) {
	float posx = x * this->x;
	float posy = y * this->y;
	int xx = roundi(posx);
	int yy = roundi(posy);
	xx = std::min(std::max(0, xx), this->x - 1);
	yy = std::min(std::max(0, yy), this->y - 1);
	return pixel(xx, yy);
}

Image* Image::scale(int width, int height) {
	unsigned* data = new unsigned[width * height];
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			data[y * width + x] = sample((float)x / (float)width, (float)y / (float)height);
		}
	}
	return new Image(width, height, data);
}
