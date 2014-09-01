#include <imagew.h>
#include "Preprocessor.h"

unsigned toValue(unsigned char* pixel) {
	unsigned char r = pixel[0]; 
	unsigned char g = pixel[1];
	unsigned char b = pixel[2];
	unsigned char a = pixel[3];
	return (r << 24) | (g << 16) | (b << 8) | a;
}

void transparent(iw_context* context, unsigned color) {
	iw_image image;
	iw_get_output_image(context, &image);

	for (int y = image.height - 1; y >= 0; --y) {
		for (int x = 0; x < image.width; ++x) {
			unsigned value = toValue(&image.pixels[y * image.bpr + x * 4]);
			if (value == color) {
				image.pixels[y * image.bpr + x * 4 + 3] = 0;
			}
		}
	}
}

void prealpha(iw_context* context) {
	iw_image image;
	iw_get_output_image(context, &image);

	for (int y = image.height - 1; y >= 0; --y) {
		for (int x = 0; x < image.width; ++x) {
			double opacity = image.pixels[y * image.bpr + x * 4 + 3] / 255.0;
			for (int i = 0; i < 3; ++i) {
				image.pixels[y * image.bpr + x * 4 + i] = static_cast<unsigned char>(image.pixels[y * image.bpr + x * 4 + i] * opacity);
			}
		}
	}
}
