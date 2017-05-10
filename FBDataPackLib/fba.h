/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#include <string>
#include <unordered_map>
#include "fba_header.h"
#include "FBCommonHeaders/Types.h"
#define FB_FBA_IGNORE_CASE 1

namespace fb {
	struct fba_file_header {
		char type[2];
		unsigned version;
		unsigned resource_version;
		unsigned passwd_hash;

		bool is_valid_fba_type() const;
		bool is_valid_fba_patch_type() const;
		bool is_valid_password(const std::string& password) const;
	};

	using fba_headers = std::vector< fba_header >;
	struct pack_datum {
		std::string pack_path; // Data/actors.fba
		unsigned version;
		unsigned resource_version;
		unsigned passwd_hash;
		fba_headers headers;
		std::unordered_map<std::string, unsigned> name_index;
#if FB_FBA_IGNORE_CASE
		std::unordered_map<std::string, unsigned> namelower_index;
#endif
	};

	// for building
	struct folder_data {
		std::string name;
		unsigned original_size;
		unsigned compressed_size;
		float compression_ratio;
	};

	bool pack_data_folder(const std::string& target_folder, const std::string& source_folder,
		unsigned resource_version, const std::string& password,
		const std::string& ignore_file, const StringVector& includeOnly,
		bool perform_validation, unsigned& out_original_size, unsigned& out_compressed_size,
		std::vector<folder_data>& out_folders_data);

	enum PackFolderResult {
		PFR_ERROR,
		PFR_SUCCESS,
		PFR_EXCLUDED,
	};
	/// folderName usually the direct child folders.
	/// for example, if you want to compress your_game_folder/data/actors:
	/// The working directory should be your_game_folder/data
	/// and the 'folder_name' should be actors
	/// The output file path will be your_game_folder/data/actors.fba
	/// return 0 if there is an error.
	/// return 1 if success.
	/// return 2 if the folder is excluded.
	PackFolderResult pack_data_folder(const std::string& folder_name, unsigned resource_version, const std::string& password,
		const std::string& ignore_file, const StringVector& includeOnly, 
		bool perform_validation, unsigned& out_original_size, unsigned& out_compressed_size);

	bool parse_fba_headers(const char* path, pack_datum& data);

	ByteArrayPtr parse_fba_data(const char* fba_path, const fba_header& header, const std::string& password);
	ByteArrayPtr parse_fba_data(std::istream& stream, const fba_header& header, const std::string& password);

	bool create_patch(const std::string& target_folder, const std::string& source_folder, const std::string& password,
		const std::string& ignore_file, bool date_only, bool perform_validation);
	bool create_patch_for_fba(const std::string& fba_name_only, const std::string& source_folder, const std::string& password,
		const std::string& ignore_file, bool date_only, bool perform_validation);

	bool apply_patch(const std::string& target_folder, const std::string& source_folder, 
		const std::string& password, bool validation);
	bool apply_patch_individual(const std::string& dest_file, const std::string& source_file, 
		const std::string& password, bool backup);
	

	/// Remove file in fba pack.
	/// Use this function when you remove serveral files from the fba pack.
	/// This doesn't change the pack file size. you need to decreasing it after saving headers. Use FileSystem::ResizeFile().
	/// \a fba_file_path Modifying fba pack
	/// \a stream fba_file_path's stream. you should open it with fba_file_path before calling this function	
	bool remove_file(std::fstream& stream, fba_headers& headers, const fba_header& header, const std::string& password, unsigned& data_end_pos);

	/// Remove file in fba pack.
	/// Use this function when you remove a file from the fba pack.
	bool remove_file(const char* fba_file_path, const char* removing_file, const std::string& password);
	enum fba_verify_type {
		verify_uncompress,
		verify_only_in_pack,
		verify_bidirectional,
	};
	bool verify_fba_to_folder(const std::string& fba_path, const std::string& folder_path,
		const std::string& password, fba_verify_type vtype);

	bool extract_fba(const char* fba_file, const std::string& to, const std::string& password);
}
