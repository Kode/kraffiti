#pragma once

#include "ByteStream.h"
//#include "Path.h"

namespace kake {
	class Image {
	public:
		Image(byte* data, int size);
		//Image(kmd::Path path);
		int width();
		int height();
		Image* scale(int width, int height);
		unsigned sample(float x, float y);
		void replaceAlpha(unsigned rgba);
		unsigned abgr(int x, int y);
		unsigned rgba(int x, int y);
		unsigned argb(int x, int y);
		unsigned pixel(int x, int y);
		byte r(int x, int y);
		byte g(int x, int y);
		byte b(int x, int y);
		byte a(int x, int y);
		//void save(kmd::Path path);
		void save(ByteStream& stream);
	private:
		Image(int x, int y, unsigned* data);
		int x, y;
		unsigned* data;
	};
}
