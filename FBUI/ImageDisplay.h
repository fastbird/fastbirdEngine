#pragma once
namespace fb{
	namespace ImageDisplay{
		enum Enum{
			// Scale image,
			FreeScaleImageMatchAll,
			KeepImageRatioMatchWidth,
			KeepImageRatioMatchHeight,			
			// Scale ui
			FreeScaleUIMatchAll,
			KeepUIRatioMatchWidth,
			KeepUIRatioMatchHeight,

			NoScale,


			Num
		};

		const char* ConvertToString(Enum display);
		Enum ConvertToEnum(const char* sz);
	}
}