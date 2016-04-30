#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(File);
	class FB_DLL_FILESYSTEM File {
		FB_DECLARE_PIMPL_NON_COPYABLE(File);
		File(const char* path);
		~File();

	public:
		static FilePtr From(const char* path);
		typedef std::vector<FilePtr> PtrArray;
		bool Exists() const;	
		FilePtr Parent() const;
		std::string Absolute() const;
		bool IsDir() const;
		bool MakeDir() const;
		bool operator==(const File& other) const;
		const std::string& Path() const;
		const char* c_str() const;
		PtrArray ListFiles() const;
		void DeleteOnExit();
		bool HasExtension(const char* ext);
		time_t LastModified() const;
		std::unique_ptr<char[]> LoadBinary(size_t& size) const;
	};
}
