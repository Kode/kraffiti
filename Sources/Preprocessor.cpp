#include <imagew.h>
#include "Preprocessor.h"

void transparent(iw_context* context, unsigned color) {
	iw_image image;
	iw_get_output_image(context, &image);

	for (int y = image.height - 1; y >= 0; --y) {
		for (int x = 0; x < image.width; ++x) {
			unsigned char* pixel = &image.pixels[y * image.bpr + x * 4];

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

			if (isred && isgreen && isblue) {
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
