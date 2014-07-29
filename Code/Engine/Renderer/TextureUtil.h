#pragma once

#include <FreeImage.h>

namespace fastbird
{	
	FIBITMAP* LoadImageFile(const char* path, int flag);
}