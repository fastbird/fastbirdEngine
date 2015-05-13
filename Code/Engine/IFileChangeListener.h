#pragma once

namespace fastbird
{
	//--------------------------------------------------------------------------------
	class IFileChangeListener
	{
	public:
		virtual bool OnFileChanged(const char* file) = 0;
	};
}