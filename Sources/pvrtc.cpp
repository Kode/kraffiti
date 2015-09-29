#include "pvrtc.h"

#ifdef LIB_PVRTC

#include <PVRTextureUtilities.h>

namespace {
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

	void writePVRTC(Image image, const char* filename) {
		int w = getPower2(image.width);
		int h = getPower2(image.height);
		w = h = imax(w, h);
		unsigned char* pixels = new unsigned char[w * h * 4];
		for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
			pixels[y * w * 4 + x * 4 + 0] = 0;
			pixels[y * w * 4 + x * 4 + 1] = 0;
			pixels[y * w * 4 + x * 4 + 2] = 0;
			pixels[y * w * 4 + x * 4 + 3] = 0;
		}
		for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x) {
			pixels[y * w * 4 + x * 4 + 0] = image.pixels[y * image.stride + x * 4 + 0];
			pixels[y * w * 4 + x * 4 + 1] = image.pixels[y * image.stride + x * 4 + 1];
			pixels[y * w * 4 + x * 4 + 2] = image.pixels[y * image.stride + x * 4 + 2];
			pixels[y * w * 4 + x * 4 + 3] = image.pixels[y * image.stride + x * 4 + 3];
		}
		pvrtexture::CPVRTextureHeader header(pvrtexture::PVRStandard8PixelType.PixelTypeID, w, h);
		pvrtexture::CPVRTexture texture(header, pixels);
		pvrtexture::Transcode(texture, ePVRTPF_PVRTCI_4bpp_RGBA, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
		
		MetaDataBlock widthBlock;
		widthBlock.DevFOURCC = 0;
		widthBlock.u32Key = 0;
		widthBlock.u32DataSize = 4;
		widthBlock.Data = new unsigned char[4];
		unsigned char* ww = (unsigned char*)&image.width;
		for (int i = 0; i < 4; ++i) widthBlock.Data[i] = ww[i];
		texture.addMetaData(widthBlock);
		
		MetaDataBlock heightBlock;
		heightBlock.DevFOURCC = 1;
		heightBlock.u32Key = 0;
		heightBlock.u32DataSize = 4;
		heightBlock.Data = new unsigned char[4];
		unsigned char* hh = (unsigned char*)&image.height;
		for (int i = 0; i < 4; ++i) heightBlock.Data[i] = hh[i];
		texture.addMetaData(heightBlock);
		
		texture.saveFile(filename);
		delete[] pixels;
	}
}

void pvrtc(Image image, const char* filename) {
	writePVRTC(image, filename);
}

#else

void pvrtc(Image image, const char* filename) {
	
}

#endif
