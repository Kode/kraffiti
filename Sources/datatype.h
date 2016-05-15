#pragma once

typedef char* (*FormatsType)();
typedef void (*EncodeType)();

struct Datatype {
	FormatsType formats;
	EncodeType encode;
};

Datatype loadDatatype(const char* name);
