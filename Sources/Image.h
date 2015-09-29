#pragma once

typedef unsigned char byte;

struct Image {
	Image(byte* pixels, int width, int height) : pixels(pixels), width(width), height(height), stride(width * 4) {

	}

	byte* pixels;
	int width;
	int height;
	int stride;
};
