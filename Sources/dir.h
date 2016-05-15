#pragma once

struct Directory {
	void* handle;
};

struct File {
	bool valid;
	char name[256];
};

Directory openDir(const char* dirname);
File readNextFile(const Directory& dir);
void closeDir(const Directory& dir);
