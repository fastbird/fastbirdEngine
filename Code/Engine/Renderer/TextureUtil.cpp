#include <Engine/StdAfx.h>
#include <Engine/Renderer/TextureUtil.h>

namespace fastbird
{

FIBITMAP* LoadImageFile(const char* path, int flag)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(path, 0);
	if(fif == FIF_UNKNOWN) 
	{
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(path);
	}
	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) 
	{
		// ok, let's load the file
		FIBITMAP *dib = FreeImage_Load(fif, path, flag);
		// unless a bad file format, we are done !
		return dib;
	}
	return NULL;
}

}