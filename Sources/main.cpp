#include "Icons.h"
#include "astc.h"
#include "pvrtc.h"
#include "Preprocessor.h"
#include <imagew.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace {
	unsigned char input_initial_bytes[12];
	size_t input_initial_bytes_stored;
	size_t input_initial_bytes_consumed;

	int my_readfn(struct iw_context *ctx, struct iw_iodescr *iodescr, void *buf, size_t nbytes, size_t *pbytesread) {
		size_t recorded_bytes_remaining;
		struct params_struct *p = (struct params_struct *)iw_get_userdata(ctx);

		recorded_bytes_remaining = input_initial_bytes_stored - input_initial_bytes_consumed;
		if (recorded_bytes_remaining > 0) {
			if (recorded_bytes_remaining >= nbytes) {
				// The read can be satisfied from the recorded bytes.
				memcpy(buf, &input_initial_bytes[input_initial_bytes_consumed], nbytes);
				input_initial_bytes_consumed += nbytes;
				*pbytesread = nbytes;
				return 1;
			}
			else {
				size_t bytes_to_read_from_file;
				size_t bytes_read_from_file;

				// Need to use some recorded bytes ...
				memcpy(buf, &input_initial_bytes[input_initial_bytes_consumed], recorded_bytes_remaining);
				input_initial_bytes_consumed += recorded_bytes_remaining;

				// ... and read the rest from the file.
				bytes_to_read_from_file = nbytes - recorded_bytes_remaining;
				bytes_read_from_file = fread(&((unsigned char*)buf)[recorded_bytes_remaining], 1, bytes_to_read_from_file, (FILE*)iodescr->fp);
				*pbytesread = recorded_bytes_remaining + bytes_read_from_file;
				return 1;
			}
		}

		*pbytesread = fread(buf, 1, nbytes, (FILE*)iodescr->fp);
		return 1;
	}

	int my_getfilesizefn(struct iw_context *ctx, struct iw_iodescr *iodescr, iw_int64 *pfilesize) {
		int ret;
		long lret;
		struct params_struct *p = (struct params_struct *)iw_get_userdata(ctx);

		FILE *fp = (FILE*)iodescr->fp;

		// TODO: Rewrite this to support >4GB file sizes.
		ret = fseek(fp, 0, SEEK_END);
		if (ret != 0) return 0;
		lret = ftell(fp);
		if (lret < 0) return 0;
		*pfilesize = (iw_int64)lret;
		fseek(fp, (long)input_initial_bytes_stored, SEEK_SET);
		return 1;
	}

	int my_seekfn(struct iw_context *ctx, struct iw_iodescr *iodescr, iw_int64 offset, int whence) {
		FILE *fp = (FILE*)iodescr->fp;
		fseek(fp, (long)offset, whence);
		return 1;
	}

	int my_writefn(struct iw_context *ctx, struct iw_iodescr *iodescr, const void *buf, size_t nbytes) {
		fwrite(buf, 1, nbytes, (FILE*)iodescr->fp);
		return 1;
	}
}

bool startsWith(std::string a, std::string b) {
	return a.substr(0, b.size()) == b;
}

bool endsWith(std::string a, std::string b) {
	return a.substr(a.size() - b.size(), b.size()) == b;
}

void writeImage(iw_context* context, const char* filename, int width, int height, int fmt) {
	iw_iodescr writedescr;
	memset(&writedescr, 0, sizeof(struct iw_iodescr));
	writedescr.write_fn = my_writefn;
	writedescr.seek_fn = my_seekfn;
	writedescr.fp = (void*)fopen(filename, "wb");
	iw_write_file_by_fmt(context, &writedescr, fmt);
	fclose((FILE*)writedescr.fp);
}

int main(int argc, char** argv) {
	std::string from = "ball.png";
	std::string to = "output.png";
	std::string format = "png";
	bool pointSampling = false;
	bool doprealpha = false;
	bool dobackground = false;
	bool dotransparency = false;
	bool keepaspect = false;
	unsigned backgroundColor = 0;
	unsigned transparentColor = 0;
	int width = -1;
	int height = -1;
	int scale = 1;

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
			scale = atoi(substring.c_str());
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
		else {
			// Unknown parameter
		}
	}

	iw_context* context = iw_create_context(NULL);

	iw_iodescr readdescr;
	memset(&readdescr, 0, sizeof(struct iw_iodescr));
	readdescr.read_fn = my_readfn;
	readdescr.getfilesize_fn = my_getfilesizefn;
	readdescr.fp = (void*)fopen(from.c_str(), "rb");
	iw_read_file_by_fmt(context, &readdescr, endsWith(from, ".png") ? IW_FORMAT_PNG : IW_FORMAT_JPEG);
	fclose((FILE*)readdescr.fp);

	iw_set_allow_opt(context, IW_OPT_PALETTE, 0);
	iw_set_allow_opt(context, IW_OPT_STRIP_ALPHA, 0);
	iw_set_allow_opt(context, IW_OPT_GRAYSCALE, 0);
	iw_set_allow_opt(context, IW_OPT_BINARY_TRNS, 0);
	int originalWidth = iw_get_value(context, IW_VAL_INPUT_WIDTH);
	int originalHeight = iw_get_value(context, IW_VAL_INPUT_HEIGHT);
	if (scale != 1) {
		width = originalWidth * scale;
		height = originalHeight * scale;
	}
	if (width < 0) width = originalWidth;
	if (height < 0) height = originalHeight;

	if (dobackground) {
		iw_color background;
		background.c[IW_CHANNELTYPE_RED] = ((backgroundColor & 0xff000000) >> 24) / 255.0;
		background.c[IW_CHANNELTYPE_GREEN] = ((backgroundColor & 0xff0000) >> 16) / 255.0;
		background.c[IW_CHANNELTYPE_BLUE] = ((backgroundColor & 0xff00) >> 8) / 255.0;
		background.c[IW_CHANNELTYPE_ALPHA] = (backgroundColor & 0xff) / 255.0;
		iw_set_apply_bkgd_2(context, &background);
	}

	if (pointSampling) {
		iw_set_resize_alg(context, 0, IW_RESIZETYPE_NEAREST, 0, 0, 0);
		iw_set_resize_alg(context, 1, IW_RESIZETYPE_NEAREST, 0, 0, 0);
	}

	if (format == "png" || format == "pvrtc" || format == "astc" || format == "jpg" || format == "jpeg") {
		if (format == "jpg" || format == "jpeg") {
			iw_set_output_profile(context, iw_get_profile_by_fmt(IW_FORMAT_JPEG) & ~IW_PROFILE_16BPS);
		}
		else {
			iw_set_output_profile(context, iw_get_profile_by_fmt(IW_FORMAT_PNG) & ~IW_PROFILE_16BPS);
		}
		iw_set_output_depth(context, 8);
		//figure_out_size_and_density(p, context);
		iw_set_output_canvas_size(context, width, height);
		double w = width;
		double h = height;
		double ow = originalWidth;
		double oh = originalHeight;
		if (keepaspect && w / h != ow / oh) {
			double scale = 1;
			if (ow / oh > w / h) {
				scale = w / ow;
			}
			else {
				scale = h / oh;
			}
			iw_set_value_dbl(context, IW_VAL_TRANSLATE_X, w / 2.0 - ow * scale / 2.0);
			iw_set_value_dbl(context, IW_VAL_TRANSLATE_Y, h / 2.0 - oh * scale / 2.0);
			iw_set_output_image_size(context, ow * scale, oh * scale);
		}
		iw_process_image(context);
	}

	if (dotransparency) {
		transparent(context, transparentColor);
	}

	if (doprealpha && !dobackground) {
		prealpha(context);
	}

	if (format == "png") {
		writeImage(context, to.c_str(), width, height, IW_FORMAT_PNG);
	}
	else if (format == "jpg" || format == "jpeg") {
		writeImage(context, to.c_str(), width, height, IW_FORMAT_JPEG);
	}
	else if (format == "ico") {
		windowsIcon(context, to.c_str());
	}
	else if (format == "icns") {
		macIcon(context, to.c_str());
	}
	else if (format == "astc") {
		astc(context, to.c_str());
	}
	else if (format == "pvrtc") {
		pvrtc(context, to.c_str());
	}
	else {
		// Unknown format
	}
}
