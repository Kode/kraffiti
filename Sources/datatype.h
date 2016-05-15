#pragma once

typedef char* (*FormatsType)();
typedef void (*EncodeType)(int width, int height, int stride, int format, unsigned char* pixels_, int* out_width, int* out_height, int* out_size, void** out_data);

struct Datatype {
	FormatsType formats;
	EncodeType encode;
};

Datatype loadDatatype(const char* name);
