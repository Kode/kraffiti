#pragma once

#include <imagew.h>

enum Color {
	white, black, transparent
};
	
void windowsIcon(iw_context* context, const char* filename);
void macIcon(iw_context* context, const char* filename);
