#pragma once

typedef unsigned char byte;

struct Image {
	Image(byte* pixels, int width, int height, int components = 4) : pixels(pixels), width(width), height(height), components(components), stride(width * components), isHdr(false) {

	}

	byte* pixels;
	int width;
	int height;
	int components;
	int stride;
	bool isHdr;
	float* hdrPixels;
};
