#include "Preprocessor.h"
#include <stdlib.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

Image transparent(Image image, unsigned color) {
	Image result((byte*)malloc(image.stride * image.height), image.width, image.height);

	for (int y = image.height - 1; y >= 0; --y) {
		for (int x = 0; x < image.width; ++x) {
			unsigned char* pixel = &image.pixels[y * image.stride + x * 4];

			unsigned char r = pixel[0];
			unsigned char g = pixel[1];
			unsigned char b = pixel[2];
			unsigned char a = pixel[3];

			unsigned char red = (color & 0xff000000) >> 24;
			unsigned char green = (color & 0xff0000) >> 16;
			unsigned char blue = (color & 0xff00) >> 8;

			bool isred = r == red || r == red - 1 || r == red + 1;
			bool isgreen = g == green || g == green - 1 || g == green + 1;
			bool isblue = b == blue || b == blue - 1 || b == blue + 1;

			result.pixels[y * image.stride + x * 4 + 0] = image.pixels[y * image.stride + x * 4 + 0];
			result.pixels[y * image.stride + x * 4 + 1] = image.pixels[y * image.stride + x * 4 + 1];
			result.pixels[y * image.stride + x * 4 + 2] = image.pixels[y * image.stride + x * 4 + 2];
			if (isred && isgreen && isblue) {
				result.pixels[y * image.stride + x * 4 + 3] = 0;
			}
			else {
				result.pixels[y * image.stride + x * 4 + 3] = image.pixels[y * image.stride + x * 4 + 3];
			}
		}
	}
	return result;
}

Image prealpha(Image image) {
	Image result((byte*)malloc(image.stride * image.height), image.width, image.height);

	for (int y = image.height - 1; y >= 0; --y) {
		for (int x = 0; x < image.width; ++x) {
			double opacity = image.pixels[y * image.stride + x * 4 + 3] / 255.0;
			for (int i = 0; i < 3; ++i) {
				result.pixels[y * image.stride + x * 4 + i] = static_cast<unsigned char>(image.pixels[y * image.stride + x * 4 + i] * opacity);
			}
			result.pixels[y * image.stride + x * 4 + 3] = image.pixels[y * image.stride + x * 4 + 3];
		}
	}
	return result;
}

Image scale(Image image, int width, int height, bool pointsample) {
	if (!image.isHdr) {
		Image result((byte*)malloc(width * height * image.components), width, height, image.components);
		
		stbir_resize_uint8_generic(image.pixels, image.width, image.height, image.stride,
								   result.pixels, result.width, result.height, result.stride,
								   image.components, 3, 0,
								   STBIR_EDGE_CLAMP, pointsample ? STBIR_FILTER_BOX : STBIR_FILTER_CUBICBSPLINE, STBIR_COLORSPACE_SRGB,
								   0);
		
		return result;
	}
	else {
		Image result(NULL, width, height, image.components);
		result.isHdr = true;
		result.hdrPixels = (float*)malloc(width * height * image.components * sizeof(float));

		stbir_resize_float(image.hdrPixels, image.width, image.height, image.width * image.components * 4,
						   result.hdrPixels, result.width, result.height, result.width * image.components * 4,
						   image.components);

		return result;
	}
}

Image scaleKeepAspect(Image image, int width, int height, bool pointsample) {
	double w = width;
	double h = height;
	double ow = image.width;
	double oh = image.height;
	
	if (w / h == ow / oh) {
		return scale(image, width, height, pointsample);
	}

	Image result((byte*)malloc(width * height * 4), width, height);

	for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
		result.pixels[y * result.stride + x * 4 + 0] = 0;
		result.pixels[y * result.stride + x * 4 + 1] = 0;
		result.pixels[y * result.stride + x * 4 + 2] = 0;
		result.pixels[y * result.stride + x * 4 + 3] = 0;
	}
	
	double scale = 1;
	if (ow / oh > w / h) {
		scale = w / ow;
	}
	else {
		scale = h / oh;
	}
	Image scaled = ::scale(image, static_cast<int>(ow * scale), static_cast<int>(oh * scale), pointsample);
	for (int y = 0; y < scaled.height; ++y) for (int x = 0; x < scaled.width; ++x) {
		int oy = static_cast<int>(h / 2.0 - oh * scale / 2.0 + y);
		int ox = static_cast<int>(w / 2.0 - ow * scale / 2.0 + x);
		result.pixels[oy * result.stride + ox * 4 + 0] = scaled.pixels[y * scaled.stride + x * 4 + 0];
		result.pixels[oy * result.stride + ox * 4 + 1] = scaled.pixels[y * scaled.stride + x * 4 + 1];
		result.pixels[oy * result.stride + ox * 4 + 2] = scaled.pixels[y * scaled.stride + x * 4 + 2];
		result.pixels[oy * result.stride + ox * 4 + 3] = scaled.pixels[y * scaled.stride + x * 4 + 3];
	}

	return result;
}

namespace {
	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int getPower2(int i) {
		for (int power = 0;; ++power)
			if (pow(power) >= i) return pow(power);
	}
}

Image toPowerOfTwo(Image image) {
	int width = getPower2(image.width);
	int height = getPower2(image.height);
	Image result((byte*)malloc(width * height * 4), width, height);

	for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
		result.pixels[y * result.stride + x * 4 + 0] = 0;
		result.pixels[y * result.stride + x * 4 + 1] = 0;
		result.pixels[y * result.stride + x * 4 + 2] = 0;
		result.pixels[y * result.stride + x * 4 + 3] = 0;
	}
	for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x) {
		result.pixels[y * result.stride + x * 4 + 0] = image.pixels[y * image.stride + x * 4 + 0];
		result.pixels[y * result.stride + x * 4 + 1] = image.pixels[y * image.stride + x * 4 + 1];
		result.pixels[y * result.stride + x * 4 + 2] = image.pixels[y * image.stride + x * 4 + 2];
		result.pixels[y * result.stride + x * 4 + 3] = image.pixels[y * image.stride + x * 4 + 3];
	}

	return result;
}
