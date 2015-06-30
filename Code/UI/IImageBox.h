#pragma once

namespace fastbird
{
class CLASS_DECLSPEC_UI IImageBox
{
public:
	virtual void SetImageFile(const char* file) = 0;
	// or
	virtual const Vec2I& SetTextureAtlasRegion(const char* atlas, const char* region) = 0;

	virtual void Highlight(bool enable) = 0;
};

}