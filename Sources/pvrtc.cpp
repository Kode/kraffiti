#include "pvrtc.h"
#include <PVRTextureUtilities.h>

namespace {
	void scale(iw_context* context, int width, int height) {
		/*Image* image;

		//Path customIcon = directory.resolve("icon.png");
		//if (Files::exists(customIcon)) {
		//	image = new Image(customIcon);
		//}
		//else {
		image = ball;
		//}
		Image* scaledImage = image->scale(width, height);
		if (color != transparent) scaledImage->replaceAlpha(color == white ? 0xffffff00 : 0);
		return scaledImage;*/

		iw_set_output_profile(context, IW_PROFILE_TRANSPARENCY); // iw_get_profile_by_fmt(IW_FORMAT_PNG));
		iw_set_output_depth(context, 8);
		//figure_out_size_and_density(p, context);
		iw_set_output_canvas_size(context, width, height);

		int originalWidth = iw_get_value(context, IW_VAL_INPUT_WIDTH);
		int originalHeight = iw_get_value(context, IW_VAL_INPUT_HEIGHT);
		double w = width;
		double h = height;
		double ow = originalWidth;
		double oh = originalHeight;
		if (w / h != ow / oh) {
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

	void writePVRTC(iw_image* image, const char* filename) {
		pvrtexture::CPVRTextureHeader header(pvrtexture::PVRStandard8PixelType.PixelTypeID, image->width, image->height);
		pvrtexture::CPVRTexture texture(header, image->pixels);
		pvrtexture::Transcode(texture, ePVRTPF_PVRTCI_4bpp_RGBA, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
		texture.saveFile(filename);
	}

	int imax(int a, int b) {
		return a > b ? a : b;
	}
}

void pvrtc(iw_context* context, const char* filename) {
	int w = iw_get_value(context, IW_VAL_INPUT_WIDTH);
	int h = iw_get_value(context, IW_VAL_INPUT_HEIGHT);
	scale(context, imax(w, h), imax(w, h));
	iw_image img;
	iw_get_output_image(context, &img);
	writePVRTC(&img, filename);
}
