#include "dir.h"
#include <stdio.h>
#include <stddef.h>

#ifdef _WIN32

#include <Windows.h>

Directory openDir(const char* dirname) {
	char pattern[MAX_PATH + 1];
	HMODULE hModule = GetModuleHandleW(NULL);
	GetModuleFileNameA(hModule, pattern, MAX_PATH);
	for (int i = strlen(pattern) - 1; i >= 0; --i) {
		if (pattern[i] == '\\') {
			pattern[i + 1] = 0;
			break;
		}
	}
	strcat(pattern, dirname);
	strcat(pattern, "\\*");
	WIN32_FIND_DATAA data;
	Directory dir;
	dir.handle = FindFirstFileA(pattern, &data);
	if (dir.handle == INVALID_HANDLE_VALUE) {
		printf("FindFirstFile failed (%d)\n", GetLastError());
		exit(1);
	}
	FindNextFileA(dir.handle, &data);
	return dir;
}

File readNextFile(const Directory& dir) {
	WIN32_FIND_DATAA data;
	File file;
	file.valid = FindNextFileA(dir.handle, &data) != 0;
	strcpy(file.name, data.cFileName);
	return file;
}

void closeDir(const Directory& dir) {
	FindClose(dir.handle);
}

#else

Directory openDir(const char* dirname) {
	Directory dir;
	dir.handle = NULL;
	return dir;
}

File readNextFile(const Directory& dir) {
	File file;
	file.valid = false;
	return file;
}

void closeDir(const Directory& dir) {

}

#endif
