#pragma once

#include "ByteStream.h"
#include "Image.h"

namespace kmd {
	struct Path {};
}

namespace kake {
	enum Color {
		white, black, transparent
	};
	
	class Ball {
	public:
		static Ball* the();
		Image* scale(int width, int height);
		Image* scale(int width, int height, Color color, kmd::Path directory);
		void exportTo(kmd::Path file, int width, int height, Color color, kmd::Path directory);
		void writeIcoHeader(ByteStream& stream);
		void writeIconDirEntry(ByteStream& stream, int width, int height, int offset);
		void writeBMPHeader(ByteStream& stream, int width, int height);
		void writeBMP(ByteStream& stream, Image* image);
		int getBMPSize(int width, int height);
		void exportToWindowsIcon(kmd::Path filename, kmd::Path directory);
		void exportToMacIcon(kmd::Path filename, kmd::Path directory);
	private:
		Ball();
		Image* ball;
	};
}
