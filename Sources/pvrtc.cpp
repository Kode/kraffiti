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
	
	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}
	
	int getPower2(int i) {
		for (int power = 0; ; ++power)
			if (pow(power) >= i) return pow(power);
	}

	int imax(int a, int b) {
		return a > b ? a : b;
	}

	void writePVRTC(iw_image* image, const char* filename) {
		int w = getPower2(image->width);
		int h = getPower2(image->height);
		w = h = imax(w, h);
		unsigned char* pixels = new unsigned char[w * h * 4];
		for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
			pixels[y * w * 4 + x * 4 + 0] = 0;
			pixels[y * w * 4 + x * 4 + 1] = 0;
			pixels[y * w * 4 + x * 4 + 2] = 0;
			pixels[y * w * 4 + x * 4 + 3] = 0;
		}
		for (int y = 0; y < image->height; ++y) for (int x = 0; x < image->width; ++x) {
			pixels[y * w * 4 + x * 4 + 0] = image->pixels[y * image->bpr + x * 4 + 0];
			pixels[y * w * 4 + x * 4 + 1] = image->pixels[y * image->bpr + x * 4 + 1];
			pixels[y * w * 4 + x * 4 + 2] = image->pixels[y * image->bpr + x * 4 + 2];
			pixels[y * w * 4 + x * 4 + 3] = image->pixels[y * image->bpr + x * 4 + 3];
		}
		pvrtexture::CPVRTextureHeader header(pvrtexture::PVRStandard8PixelType.PixelTypeID, w, h);
		pvrtexture::CPVRTexture texture(header, pixels);
		pvrtexture::Transcode(texture, ePVRTPF_PVRTCI_4bpp_RGBA, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
		
		MetaDataBlock widthBlock;
		widthBlock.DevFOURCC = 0;
		widthBlock.u32Key = 0;
		widthBlock.u32DataSize = 4;
		widthBlock.Data = new unsigned char[4];
		unsigned char* ww = (unsigned char*)&image->width;
		for (int i = 0; i < 4; ++i) widthBlock.Data[i] = ww[i];
		texture.addMetaData(widthBlock);
		
		MetaDataBlock heightBlock;
		heightBlock.DevFOURCC = 1;
		heightBlock.u32Key = 0;
		heightBlock.u32DataSize = 4;
		heightBlock.Data = new unsigned char[4];
		unsigned char* hh = (unsigned char*)&image->height;
		for (int i = 0; i < 4; ++i) heightBlock.Data[i] = hh[i];
		texture.addMetaData(heightBlock);
		
		texture.saveFile(filename);
		delete[] pixels;
	}
}

void pvrtc(iw_context* context, const char* filename) {
	//int w = iw_get_value(context, IW_VAL_INPUT_WIDTH);
	//int h = iw_get_value(context, IW_VAL_INPUT_HEIGHT);
	//scale(context, getPower2(w), getPower2(h));
	iw_image img;
	iw_get_output_image(context, &img);
	writePVRTC(&img, filename);
}
