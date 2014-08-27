#include "Ball.h"
//#include "Files.h"
#include <vector>

using namespace kmd;
using namespace kake;

namespace {
	const int iconHeaderSize = 6;
	const int iconDirEntrySize = 16;
	const int bmpHeaderSize = 40;
	Ball* instance = nullptr;

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

}

Ball* Ball::the() {
	if (instance == nullptr) instance = new Ball;
	return instance;
}
	
Image* Ball::scale(int width, int height) {
	return scale(width, height, transparent, Path()); //Paths::get("."));
}
	
Image* Ball::scale(int width, int height, Color color, Path directory) {
	Image* image;
	
	//Path customIcon = directory.resolve("icon.png");
	//if (Files::exists(customIcon)) {
	//	image = new Image(customIcon);
	//}
	//else {
		image = ball;
	//}
	Image* scaledImage = image->scale(width, height);
	if (color != transparent) scaledImage->replaceAlpha(color == white ? 0xffffff00 : 0);
	return scaledImage;
}
	
void Ball::exportTo(Path file, int width, int height, Color color, Path directory) {
	scale(width, height, color, directory); // ->save(file);
}

void Ball::writeIcoHeader(ByteStream& stream) {
	stream.put(0); stream.put(0); //Reserved. Must always be 0.
	stream.write(convertShortToByteArrayLE((short)1)); //Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
	stream.write(convertShortToByteArrayLE((short)4)); //Specifies number of images in the file.
}

void Ball::writeIconDirEntry(ByteStream& stream, int width, int height, int offset) {
	stream.put(width == 256 ? 0 : width); //Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
	stream.put(height == 256 ? 0 : height); //Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is 256 pixels.
	stream.put(0); //Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
	stream.put(0); //Reserved. Should be 0.[Notes 2]
	stream.write(convertShortToByteArrayLE((short)1)); //Specifies color planes. Should be 0 or 1.[Notes 3]
	stream.write(convertShortToByteArrayLE((short)32)); //Specifies bits per pixel. [Notes 4]
	stream.write(convertIntToByteArrayLE(width * height * 4 + 40)); //Specifies the size of the image's data in bytes
	stream.write(convertIntToByteArrayLE(offset)); //Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file
}

void Ball::writeBMPHeader(ByteStream& stream, int width, int height) {
	stream.write(convertIntToByteArrayLE(bmpHeaderSize)); //the size of this header (40 bytes)
	stream.write(convertIntToByteArrayLE(width)); //the bitmap width in pixels (signed integer).
	stream.write(convertIntToByteArrayLE(height * 2)); //the bitmap height in pixels (signed integer).
	stream.write(convertShortToByteArrayLE((short)1)); //the number of color planes being used. Must be set to 1.
	stream.write(convertShortToByteArrayLE((short)32)); //the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32.
	stream.write(convertIntToByteArrayLE(0)); //the compression method being used. See the next table for a list of possible values.
	stream.write(convertIntToByteArrayLE(width * height * 4)); //the image size. This is the size of the raw bitmap data (see below), and should not be confused with the file size.
	stream.write(convertIntToByteArrayLE(0)); //the horizontal resolution of the image. (pixel per meter, signed integer)
	stream.write(convertIntToByteArrayLE(0)); //the vertical resolution of the image. (pixel per meter, signed integer)
	stream.write(convertIntToByteArrayLE(0)); //the number of colors in the color palette, or 0 to default to 2n.
	stream.write(convertIntToByteArrayLE(0)); //the number of important colors used, or 0 when every color is important; generally ignored.
}
	
void Ball::writeBMP(ByteStream& stream, Image* image) {
	writeBMPHeader(stream, image->width(), image->height());
	for (int y = image->height() - 1; y >= 0; --y) {
		for (int x = 0; x < image->width(); ++x) {
			//stream.put(image->b(x, y));
			//stream.put(image->g(x, y));
			//stream.put(image->r(x, y));
			//stream.put(image->a(x, y));
			stream.write(convertIntToByteArrayLE(image->argb(x, y)));
		}
	}
}

int Ball::getBMPSize(int width, int height) {
	return width * height * 4 + bmpHeaderSize;
}
	
void Ball::exportToWindowsIcon(Path filename, Path directory) {
	//16x16
	//32x32
	//48x48
	//256x256

	ByteStream stream;
	writeIcoHeader(stream);

	writeIconDirEntry(stream, 16, 16, iconHeaderSize + iconDirEntrySize * 4);
	writeIconDirEntry(stream, 32, 32, iconHeaderSize + iconDirEntrySize * 4 + getBMPSize(16, 16));
	writeIconDirEntry(stream, 48, 48, iconHeaderSize + iconDirEntrySize * 4 + getBMPSize(16, 16) + getBMPSize(32, 32));
	writeIconDirEntry(stream, 256, 256, iconHeaderSize + iconDirEntrySize * 4 + getBMPSize(16, 16) + getBMPSize(32, 32) + getBMPSize(48, 48));

	writeBMP(stream, scale(16, 16, transparent, directory));
	writeBMP(stream, scale(32, 32, transparent, directory));
	writeBMP(stream, scale(48, 48, transparent, directory));

	scale(256, 256, transparent, directory)->save(stream);
			
	std::vector<byte> pngSize = convertIntToByteArrayLE(static_cast<int>(stream.size()) - (iconHeaderSize + iconDirEntrySize * 4 + getBMPSize(16, 16) + getBMPSize(32, 32) + getBMPSize(48, 48)));
	for (int i = 0; i < 4; ++i) stream.set(i + iconHeaderSize + iconDirEntrySize * 3 + 8, pngSize[i]);
	
	//stream.save(filename);
}
	
void Ball::exportToMacIcon(Path filename, Path directory) {
	//16x16
	//32x32
	//128x128
	//256x256
	//512x512
	//1024x1024

	ByteStream stream;
	stream.put('i'); stream.put('c'); stream.put('n'); stream.put('s');
	stream.put('-'); stream.put('-'); stream.put('-'); stream.put('-');
			
	stream.put('i'); stream.put('c'); stream.put('0'); stream.put('8');
	stream.put('-'); stream.put('-'); stream.put('-'); stream.put('-');
	scale(256, 256, transparent, directory)->save(stream);
	int icon08size = static_cast<int>(stream.size() - 8);
	stream.put('i'); stream.put('c'); stream.put('0'); stream.put('9');
	stream.put('-'); stream.put('-'); stream.put('-'); stream.put('-');
	scale(512, 512, transparent, directory)->save(stream);
	int icon09size = static_cast<int>(stream.size() - icon08size - 8);
	stream.put('i'); stream.put('c'); stream.put('1'); stream.put('0');
	stream.put('-'); stream.put('-'); stream.put('-'); stream.put('-');
	scale(1024, 1024, transparent, directory)->save(stream);
	int icon10size = static_cast<int>(stream.size() - icon09size - icon08size - 8);
	
	std::vector<byte> size = convertIntToByteArray(static_cast<int>(stream.size()));
	for (int i = 0; i < 4; ++i) stream.set( 4 + i, size[i]);
	
	size = convertIntToByteArray(icon08size);
	for (int i = 0; i < 4; ++i) stream.set(12 + i, size[i]);
	
	size = convertIntToByteArray(icon09size);
	for (int i = 0; i < 4; ++i) stream.set(icon08size + 8 + 4 + i, size[i]);
	
	size = convertIntToByteArray(icon10size);
	for (int i = 0; i < 4; ++i) stream.set(icon08size + icon09size + 8 + 4 + i, size[i]);
	
	//stream.save(filename);
}

//#include "hexball.h"

Ball::Ball() {
//	ball = new Image(hex_array, sizeof(hex_array));
	ball = nullptr;
}
