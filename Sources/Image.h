#pragma once

typedef unsigned char byte;

struct Image {
	Image(byte* pixels, int width, int height, int pixelStride = 4) : pixels(pixels), width(width), height(height), pixelStride(pixelStride), stride(width * pixelStride) {

	}

	byte* pixels;
	int width;
	int height;
	int pixelStride;
	int stride;
};
