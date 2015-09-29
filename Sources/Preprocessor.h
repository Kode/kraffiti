#pragma once

#include "Image.h"

Image transparent(Image image, unsigned color);
Image prealpha(Image image);
Image scale(Image image, int width, int height, bool pointsample);
Image scaleKeepAspect(Image image, int width, int height, bool pointsample);
Image toPowerOfTwo(Image image);
