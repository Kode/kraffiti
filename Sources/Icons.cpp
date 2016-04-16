#include "Icons.h"
#include "Image.h"
#include "Preprocessor.h"
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
	const int iconHeaderSize = 6;
	const int iconDirEntrySize = 16;
	const int bmpHeaderSize = 40;

	int convertByteArrayToInt(std::vector<byte> buffer) {
		if (buffer.size() != 4) throw new std::exception;

		int
		value  = (0xFF & buffer[0]) << 24;
		value |= (0xFF & buffer[1]) << 16;
		value |= (0xFF & buffer[2]) << 8;
		value |= (0xFF & buffer[3]);

		return value;
	}

	std::vector<byte> convertIntToByteArray(unsigned val) {
		std::vector<byte> buffer;
		buffer.resize(4);

		buffer[0] = (byte) (val >> 24);
		buffer[1] = (byte) (val >> 16);
		buffer[2] = (byte) (val >> 8);
		buffer[3] = (byte) val;

		return buffer;
	}

	std::vector<byte> convertIntToByteArrayLE(unsigned val) {
		std::vector<byte> buffer;
		buffer.resize(4);

		buffer[3] = (byte) (val >> 24);
		buffer[2] = (byte) (val >> 16);
		buffer[1] = (byte) (val >> 8);
		buffer[0] = (byte) val;

		return buffer;
	}

	std::vector<byte> convertShortToByteArrayLE(unsigned short val) {
		std::vector<byte> buffer;
		buffer.resize(2);

		buffer[1] = (byte) (val >> 8);
		buffer[0] = (byte) val;

		return buffer;
	}

	void write(FILE* file, const std::vector<byte>& data) {
		for (unsigned i = 0; i < data.size(); ++i) {
			fputc(data[i], file);
		}
	}
}

void writeIcoHeader(FILE* file) {
	fputc(0, file); fputc(0, file); //Reserved. Must always be 0.
	write(file, convertShortToByteArrayLE((short)1)); //Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
	write(file, convertShortToByteArrayLE((short)8)); //Specifies number of images in the file.
}

void writeIconDirEntry(FILE* file, int width, int height, int offset) {
	fputc(width == 256 ? 0 : width, file); //Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
	fputc(height == 256 ? 0 : height, file); //Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is 256 pixels.
	fputc(0, file); //Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
	fputc(0, file); //Reserved. Should be 0.[Notes 2]
	write(file, convertShortToByteArrayLE((short)1)); //Specifies color planes. Should be 0 or 1.[Notes 3]
	write(file, convertShortToByteArrayLE((short)32)); //Specifies bits per pixel. [Notes 4]
	write(file, convertIntToByteArrayLE(width * height * 4 + 40)); //Specifies the size of the image's data in bytes
	write(file, convertIntToByteArrayLE(offset)); //Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
}

void writeBMPHeader(FILE* file, int width, int height) {
	write(file, convertIntToByteArrayLE(bmpHeaderSize)); //the size of this header (40 bytes)
	write(file, convertIntToByteArrayLE(width)); //the bitmap width in pixels (signed integer).
	write(file, convertIntToByteArrayLE(height * 2)); //the bitmap height in pixels (signed integer).
	write(file, convertShortToByteArrayLE((short)1)); //the number of color planes being used. Must be set to 1.
	write(file, convertShortToByteArrayLE((short)32)); //the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32.
	write(file, convertIntToByteArrayLE(0)); //the compression method being used. See the next table for a list of possible values.
	write(file, convertIntToByteArrayLE(width * height * 4)); //the image size. This is the size of the raw bitmap data (see below), and should not be confused with the file size.
	write(file, convertIntToByteArrayLE(0)); //the horizontal resolution of the image. (pixel per meter, signed integer)
	write(file, convertIntToByteArrayLE(0)); //the vertical resolution of the image. (pixel per meter, signed integer)
	write(file, convertIntToByteArrayLE(0)); //the number of colors in the color palette, or 0 to default to 2n.
	write(file, convertIntToByteArrayLE(0)); //the number of important colors used, or 0 when every color is important; generally ignored.
}

void writeBMP(FILE* file, Image image) {
	writeBMPHeader(file, image.width, image.height);
	for (int y = image.height - 1; y >= 0; --y) {
		for (int x = 0; x < image.width; ++x) {
			//stream.put(image->b(x, y));
			//stream.put(image->g(x, y));
			//stream.put(image->r(x, y));
			//stream.put(image->a(x, y));

			//stream.write(convertIntToByteArrayLE(image->argb(x, y)));
			
			fputc(image.pixels[y * image.stride + x * 4 + 2], file);
			fputc(image.pixels[y * image.stride + x * 4 + 1], file);
			fputc(image.pixels[y * image.stride + x * 4 + 0], file);
			fputc(image.pixels[y * image.stride + x * 4 + 3], file);
		}
	}
}

int getBMPSize(int width, int height) {
	return width * height * 4 + bmpHeaderSize;
}

void windowsIcon(Image image, const char* filename) {
	FILE* file = fopen(filename, "wb");

	writeIcoHeader(file);

	int iconOffset = iconHeaderSize + iconDirEntrySize * 8;
	writeIconDirEntry(file, 16, 16, iconOffset); iconOffset += getBMPSize(16, 16);
	writeIconDirEntry(file, 20, 20, iconOffset); iconOffset += getBMPSize(20, 20);
	writeIconDirEntry(file, 24, 24, iconOffset); iconOffset += getBMPSize(24, 24);
	writeIconDirEntry(file, 32, 32, iconOffset); iconOffset += getBMPSize(32, 32);
	writeIconDirEntry(file, 40, 40, iconOffset); iconOffset += getBMPSize(40, 40);
	writeIconDirEntry(file, 48, 48, iconOffset); iconOffset += getBMPSize(48, 48);
	writeIconDirEntry(file, 64, 64, iconOffset); iconOffset += getBMPSize(64, 64);
	writeIconDirEntry(file, 256, 256, iconOffset);

	Image scaled = scaleKeepAspect(image, 16, 16, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 20, 20, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 24, 24, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 32, 32, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 40, 40, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 48, 48, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 64, 64, true);
	writeBMP(file, scaled);

	scaled = scaleKeepAspect(image, 256, 256, true);
	png_image pngimage = { 0 };
	pngimage.version = PNG_IMAGE_VERSION;
	pngimage.opaque = NULL;
	pngimage.width = scaled.width;
	pngimage.height = scaled.height;
	pngimage.format = PNG_FORMAT_RGBA;
	pngimage.flags = 0;
	png_image_write_to_stdio(&pngimage, file, 0, scaled.pixels, scaled.stride, NULL);

	std::vector<byte> pngSize = convertIntToByteArrayLE(static_cast<int>(ftell(file)) - iconOffset);
	fseek(file, iconHeaderSize + iconDirEntrySize * 7 + 8, SEEK_SET);
	for (int i = 0; i < 4; ++i) fputc(pngSize[i], file);

	fclose(file);
}

void macIcon(Image image, const char* filename) {
	//16x16
	//32x32
	//128x128
	//256x256
	//512x512
	//1024x1024

	FILE* file = fopen(filename, "wb");

	fputs("icns----ic08----", file);
	
	Image scaled = scaleKeepAspect(image, 256, 256, true);	
	png_image pngimage = { 0 };
	pngimage.version = PNG_IMAGE_VERSION;
	pngimage.opaque = NULL;
	pngimage.width = scaled.width;
	pngimage.height = scaled.height;
	pngimage.format = PNG_FORMAT_RGBA;
	pngimage.flags = 0;
	png_image_write_to_stdio(&pngimage, file, 0, scaled.pixels, scaled.stride, NULL);

	int icon08size = static_cast<int>(ftell(file) - 8);
	fputs("ic09----", file);

	scaled = scaleKeepAspect(image, 512, 512, true);
	pngimage.width = scaled.width;
	pngimage.height = scaled.height;
	png_image_write_to_stdio(&pngimage, file, 0, scaled.pixels, scaled.stride, NULL);

	int icon09size = static_cast<int>(ftell(file) - icon08size - 8);
	fputs("ic10----", file);

	scaled = scaleKeepAspect(image, 1024, 1024, true);
	pngimage.width = scaled.width;
	pngimage.height = scaled.height;
	png_image_write_to_stdio(&pngimage, file, 0, scaled.pixels, scaled.stride, NULL);

	int icon10size = static_cast<int>(ftell(file) - icon09size - icon08size - 8);

	std::vector<byte> size = convertIntToByteArray(static_cast<int>(ftell(file)));
	fseek(file, 4, SEEK_SET);
	for (int i = 0; i < 4; ++i) fputc(size[i], file);

	size = convertIntToByteArray(icon08size);
	fseek(file, 12, SEEK_SET);
	for (int i = 0; i < 4; ++i) fputc(size[i], file);

	size = convertIntToByteArray(icon09size);
	fseek(file, icon08size + 8 + 4, SEEK_SET);
	for (int i = 0; i < 4; ++i) fputc(size[i], file);

	size = convertIntToByteArray(icon10size);
	fseek(file, icon08size + icon09size + 8 + 4, SEEK_SET);
	for (int i = 0; i < 4; ++i) fputc(size[i], file);

	fclose(file);
}
