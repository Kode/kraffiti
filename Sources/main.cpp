#include "Icons.h"
#include "astc.h"
#include "pvrtc.h"
#include "Preprocessor.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <png.h>
extern "C" {
	#include <jpeglib.h>
}

bool startsWith(std::string a, std::string b) {
	return a.substr(0, b.size()) == b;
}

bool endsWith(std::string a, std::string b) {
	return a.substr(a.size() - b.size(), b.size()) == b;
}

struct my_error_mgr {
	jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr* my_error_ptr;

void my_error_exit(j_common_ptr cinfo) {
	my_error_ptr myerr = (my_error_ptr)cinfo->err;
	(*cinfo->err->output_message)(cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

Image readJPEG(const char* filename) {
	my_error_mgr jerr;
	
	FILE* infile;
	
	int row_stride;

	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return Image(NULL, 0, 0);
	}

	jpeg_decompress_struct cinfo;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return Image(NULL, 0, 0);
	}

	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	Image image((byte*)malloc(cinfo.output_width * cinfo.output_height * 4), cinfo.output_width, cinfo.output_height);
	
	row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		int scanline = cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);
		for (unsigned x = 0; x < cinfo.output_width; ++x) {
			image.pixels[scanline * image.stride + x * 4 + 0] = buffer[0][x * cinfo.num_components + 0];
			image.pixels[scanline * image.stride + x * 4 + 1] = buffer[0][x * cinfo.num_components + 1];
			image.pixels[scanline * image.stride + x * 4 + 2] = buffer[0][x * cinfo.num_components + 2];
			image.pixels[scanline * image.stride + x * 4 + 3] = 255;
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	return image;
}

void writeJPEG(Image image, const char* filename) {
	int quality = 85;

	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	
	FILE* outfile;
	JSAMPROW row_pointer[1];
	
	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image.width;
	cinfo.image_height = image.height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	byte* row = (byte*)malloc(image.width * 3);
	while (cinfo.next_scanline < cinfo.image_height) {
		for (int x = 0; x < image.width; ++x) {
			row[x * 3 + 0] = image.pixels[cinfo.next_scanline * image.stride + x * image.components + 0];
			row[x * 3 + 1] = image.pixels[cinfo.next_scanline * image.stride + x * image.components + 1];
			row[x * 3 + 2] = image.pixels[cinfo.next_scanline * image.stride + x * image.components + 2];
		}
		row_pointer[0] = row;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	free(row);

	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
}

Image readPNG(const char* filename) {
	png_image image;
	memset(&image, 0, sizeof(image));
	image.version = PNG_IMAGE_VERSION;

	if (png_image_begin_read_from_file(&image, filename)) {
		image.format = PNG_FORMAT_RGBA;
		png_bytep buffer = (png_bytep)malloc(PNG_IMAGE_SIZE(image) * 10);
		if (buffer != NULL && png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
			return Image(buffer, image.width, image.height);
		}
	}
	// error
	return Image(NULL, 0, 0);
}

void writePNG(Image image, const char* filename) {
	png_image img;
	memset(&img, 0, sizeof(image));
	img.version = PNG_IMAGE_VERSION;
	img.opaque = NULL;
	img.width = image.width;
	img.height = image.height;
	img.format = PNG_FORMAT_RGBA;
	img.flags = 0;

	if (!png_image_write_to_file(&img, filename, 0, image.pixels, 0, NULL)) {
		// error
	}
}

Image readHDR(const char* filename, bool storeHdr) {
	int width, height, n;
	if (storeHdr) {
		float *data = stbi_loadf(filename, &width, &height, &n, 0);
		Image image = Image(NULL, width, height, n);
		image.isHdr = true;
		image.hdrPixels = data;
		return image;
	}
	else {
		unsigned char *data = stbi_load(filename, &width, &height, &n, 0);
		return Image(data, width, height, n);
	}
}

void writeHDR(Image image, const char* filename) {
	float *pixels;
	if (!image.isHdr) {
		pixels = (float*)malloc(image.width * image.height * image.components * sizeof(float));
		for (int i = 0; i < image.width * image.width * image.components; ++i) {
			pixels[i] = float(image.pixels[i]) / 255;
		}
	}
	else {
		pixels = image.hdrPixels;
	}
	
	if (!stbi_write_hdr(filename, image.width, image.height, image.components, pixels)) {
		// error
	}
	
	if (!image.isHdr) {
		free(pixels);
	}
}

int main(int argc, char** argv) {
	std::string from = "ball.png";
	std::string to = "output.png";
	std::string format = "png";
	bool pointSampling = false;
	bool doprealpha = false;
	bool dobackground = false;
	bool dotransparency = false;
	bool donothing = false;
	bool keepaspect = false;
	bool topowerofto = false;
	unsigned backgroundColor = 0;
	unsigned transparentColor = 0;
	int width = -1;
	int height = -1;
	float scale = 1;

	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);

		if (startsWith(arg, "from=")) from = arg.substr(5);
		else if (startsWith(arg, "to=")) to = arg.substr(3);
		else if (startsWith(arg, "format=")) format = arg.substr(7);
		else if (startsWith(arg, "width=")) {
			std::string substring = arg.substr(6);
			width = atoi(substring.c_str());
		}
		else if (startsWith(arg, "height=")) {
			std::string substring = arg.substr(7);
			height = atoi(substring.c_str());
		}
		else if (startsWith(arg, "scale=")) {
			std::string substring = arg.substr(6);
			scale = atof(substring.c_str());
		}
		else if (startsWith(arg, "transparent=")) {
			dotransparency = true;
			std::string substring = arg.substr(12);
			transparentColor = static_cast<unsigned>(strtoll(substring.c_str(), NULL, 16));
		}
		else if (startsWith(arg, "background=")) {
			dobackground = true;
			std::string substring = arg.substr(11);
			backgroundColor = static_cast<unsigned>(strtoll(substring.c_str(), NULL, 16));
		}
		else if (arg == "prealpha") {
			doprealpha = true;
		}
		else if (startsWith(arg, "filter=")) {
			if (arg.substr(7) == "nearest") {
				pointSampling = true;
			}
		}
		else if (arg == "keepaspect") {
			keepaspect = true;
		}
		else if (arg == "poweroftwo") {
			topowerofto = true;
		}
		else if (arg == "donothing") {
			donothing = true;
		}
		else {
			// Unknown parameter
		}
	}

	if (donothing) {
		int n;
		stbi_info(from.c_str(), &width, &height, &n);
		printf("#%ix%i", width, height);
		return 0;
	}

	Image image(NULL, 0, 0);
	if (endsWith(from, ".png")) image = readPNG(from.c_str());
	else if (endsWith(from, ".hdr")) image = readHDR(from.c_str(), format == "hdr");
	else image = readJPEG(from.c_str());

	if (scale != 1) {
		width = image.width * scale;
		height = image.height * scale;
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;
	}
	
	if (width < 0) width = image.width;
	if (height < 0) height = image.height;

	
	printf("#%ix%i", width, height);

	if (dobackground) {
		for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x) {
			float alpha = image.pixels[y * image.stride + x * 4 + 3] / 255.0f;
			float red = ((backgroundColor & 0xff000000) >> 24) / 255.0f;
			float green = ((backgroundColor & 0xff0000) >> 16) / 255.0f;
			float blue = ((backgroundColor & 0xff00) >> 8) / 255.0f;
			image.pixels[y * image.stride + x * 4 + 0] = (byte)((alpha * (image.pixels[y * image.stride + x * 4 + 0] / 255.0f) + (1 - alpha) * red) * 255.0f);
			image.pixels[y * image.stride + x * 4 + 1] = (byte)((alpha * (image.pixels[y * image.stride + x * 4 + 1] / 255.0f) + (1 - alpha) * green) * 255.0f);
			image.pixels[y * image.stride + x * 4 + 2] = (byte)((alpha * (image.pixels[y * image.stride + x * 4 + 2] / 255.0f) + (1 - alpha) * blue) * 255.0f);
			image.pixels[y * image.stride + x * 4 + 3] = backgroundColor & 0xff;
		}
	}

	if (topowerofto || width != image.width || height != image.height) {
		if (keepaspect) {
			image = scaleKeepAspect(image, width, height, pointSampling);
		}
		else {
			image = ::scale(image, width, height, pointSampling);
		}
		if (topowerofto) {
			image = toPowerOfTwo(image);
		}
	}

	if (dotransparency) {
		image = transparent(image, transparentColor);
	}

	if (doprealpha && !dobackground) {
		image = prealpha(image);
	}

	if (format == "png") {
		writePNG(image, to.c_str());
	}
	else if (format == "jpg" || format == "jpeg") {
		writeJPEG(image, to.c_str());
	}
	else if (format == "hdr") {
		writeHDR(image, to.c_str());
	}
	else if (format == "ico") {
		windowsIcon(image, to.c_str());
	}
	else if (format == "icns") {
		macIcon(image, to.c_str());
	}
	else if (format == "astc") {
		astc(image, to.c_str());
	}
	else if (format == "pvrtc") {
		pvrtc(image, to.c_str());
	}
	else {
		// Unknown format
	}
}
