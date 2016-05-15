#include "datatype.h"

#ifdef SYS_WINDOWS

#include <Windows.h>

Datatype loadDatatype(const char* name) {
	HMODULE lib = LoadLibraryA(name);
	Datatype datatype;
	datatype.formats = (FormatsType)GetProcAddress(lib, "formats");
	datatype.encode = (EncodeType)GetProcAddress(lib, "encode");
	return datatype;
}

#else

Datatype loadDatatype(const char* name) {
	Datatype datatype;
	datatype.formats = NULL;
	datatype.encode = NULL;
	return datatype;
}

#endif
