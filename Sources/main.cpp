#include "Ball.h"
#include <imagew.h>
#include <memory.h>
#include <stdio.h>
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

void writeImage(iw_context* context, const char* filename, int width, int height, int fmt) {
	iw_set_output_profile(context, iw_get_profile_by_fmt(fmt) & ~IW_PROFILE_16BPS);
	iw_set_output_depth(context, 32);
	//figure_out_size_and_density(p, context);
	iw_set_output_canvas_size(context, width, height);
	iw_process_image(context);

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
	int width = 256;
	int height = 256;

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
		else {
			// Unknown parameter
		}
	}
	
	iw_context* context = iw_create_context(nullptr);

	iw_iodescr readdescr;
	memset(&readdescr, 0, sizeof(struct iw_iodescr));
	readdescr.read_fn = my_readfn;
	readdescr.getfilesize_fn = my_getfilesizefn;
	readdescr.fp = (void*)fopen(from.c_str(), "rb");
	iw_read_file_by_fmt(context, &readdescr, IW_FORMAT_PNG);
	fclose((FILE*)readdescr.fp);

	if (format == "png") {
		writeImage(context, to.c_str(), width, height, IW_FORMAT_PNG);
	}
	else if (format == "ico") {
		windowsIcon(context, to.c_str());
	}
	else if (format == "icon") {
		macIcon(context, to.c_str());
	}
	else {
		// Unknown format
	}
}
