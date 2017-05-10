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

#include <regex>
#include <set>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/level.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/scope_exit.hpp>
#include "FBStringLib/MurmurHash.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBDebugLib/DebugLib.h"
#include "FBStringLib/StringLib.h"
#include "zlib.h"
#define FB_DLL_FILESYSTEM __declspec(dllimport)
#include "FBFileSystem/FileSystem.h"
#include "fba.h"
#include "compress_uncompress.h"
// You don't have this file. Just comment it out for compiling.
#include "../fbd/fbd/fba_encrypt.h"
// If you want to implement your own ecryption algorithm, implement the following functions and add '#define FB_DATA_ENCRYPT 1'
// void encrypt_data(std::vector<char>& data, const std::string& password); /// \a data is in/out parameter. you can change the size of data in the function.
// void decrypt_data(std::vector<char>& data, const std::string& password); /// \a data is in/out parameter. you can change the size of data in the function.
// void write_encrypted_data(std::ostream& stream, const std::vector<char>& data, const std::string& password, const fba_header& h);
// void read_encrypted_data(std::istream& stream, std::vector<char>& data, const std::string& password, const fba_header& h);

namespace fb {
	char s_fba_type[2] = { (char)0xfb, (char)0xa0 };
	char s_fba_patch_type[2] = { (char)0xfb, (char)0xaa };

	bool fba_file_header::is_valid_fba_type() const {
		return memcmp(s_fba_type, type, 2) == 0;
	}

	bool fba_file_header::is_valid_fba_patch_type() const {
		return memcmp(s_fba_patch_type, type, 2) == 0;
	}

	bool fba_file_header::is_valid_password(const std::string& password) const {
		return passwd_hash == 0 || passwd_hash == murmur3_32(password.c_str(), password.size());
	}

	static std::set<std::string> ignores;

	bool ignore(const std::string& path) {
		for (auto& it : ignores) {
			std::regex e(it);
			std::smatch result;
			if (std::regex_search(path, result, e)) {
				return true;
			}
		}
		return false;
	}

	bool pack_data_folder(const std::string& target_folder, const std::string& source_folder, unsigned resource_version,
		const std::string& password, const std::string& ignore_file, const StringVector& includeOnly, 
		bool perform_validation, unsigned& total_original_size, unsigned& total_compressed_size, std::vector<folder_data>& folders_data)
	{
		total_original_size = 0;
		total_compressed_size = 0;
		auto it = FileSystem::GetDirectoryIterator(source_folder.c_str(), false);
		if(!it) {
			std::cerr << "Cannot open " << source_folder << std::endl;
			return false;
		}

		while(it->HasNext()){
			bool is_dir;
			auto path = it->GetNextFilePath(&is_dir);
			if (!is_dir)
				continue;

			unsigned original_size = 0;
			unsigned compressed_size = 0;
			auto ret = pack_data_folder(path, resource_version, password, ignore_file, includeOnly,
				perform_validation, original_size, compressed_size);
			if (ret == PFR_ERROR) {
				return false;
			}
			else if (ret == PFR_SUCCESS) {
				auto fba_path = FileSystem::GetLastDirectory(FileSystem::AddEndingSlashIfNot(path).c_str());
				fba_path += FB_FBA_EXT;
				auto target_path = FileSystem::ConcatPath(target_folder.c_str(),
					FileSystem::GetFileName(fba_path.c_str()).c_str());
				FileSystem::Rename(fba_path.c_str(), target_path.c_str());
				folders_data.push_back(folder_data{ path, original_size, compressed_size, 1.0f - (float)compressed_size / (float)original_size });
				total_original_size += original_size;
				total_compressed_size += compressed_size;
			}
		}
		return true;
	}

	PackFolderResult pack_data_folder(const std::string& data_folder, unsigned resource_version,
		const std::string& password, const std::string& ignore_file, const StringVector& includeOnly, 
		bool perform_validation, unsigned& out_original_size, unsigned& out_compressed_size)
	{
		if (data_folder.empty()) {
			std::cerr << "Invalid arg.\n";
			return PFR_ERROR;
		}

		ignores.clear();
		if (!ignore_file.empty()) {
			std::ifstream file(ignore_file);
			if (file) {
				while (file) {
					std::string item;
					file >> item;
					if (!item.empty())
						ignores.insert(item);
				}
			}
		}
		auto folder_name = FileSystem::RemoveEndingSlash(data_folder.c_str());
		auto found = folder_name.find_last_of('/');
		if (found != std::string::npos) {
			folder_name = folder_name.substr(found + 1);
		}
		if (!includeOnly.empty()) {
			bool found = false;
			for (const auto& i : includeOnly) {
				if (i == folder_name) {
					found = true;
					break;
				}
			}
			if (!found) {
				std::cout << FormatString("data folder excluded: %s\n", folder_name.c_str());
				return PFR_EXCLUDED;
			}
		}

		auto output_file_path = folder_name + FB_FBA_EXT;
		std::ofstream result_file(output_file_path.c_str(), std::ofstream::binary);
		if (!result_file) {
			std::cerr << FormatString("Failed to open the output file(%s)\n", output_file_path.c_str());
			return PFR_ERROR;
		}
		auto poar = std::make_shared<boost::archive::binary_oarchive>(result_file);
		auto& oar = *poar;
		fba_file_header file_header =
		{ s_fba_type[0], s_fba_type[1], fba_version, resource_version, password.empty() ? 0 : murmur3_32(password.c_str(), password.size()) };

		oar & file_header;

		fba_headers headers;
		unsigned index = 0;
		unsigned original_size = 0;
		unsigned compressed_size = 0;
		auto iterator = FileSystem::GetDirectoryIterator(data_folder.c_str(), true);
		while (iterator && iterator->HasNext()) {
			bool is_directory;
			auto file_path = iterator->GetNextFilePath(&is_directory);
			if (is_directory)
				continue;			
			std::string path_in_pack = file_path;
			// data_folder = "Data_original/actors"
			// folder_name = "actors"
			// file_path = "Data_original/actors/someactor.lua"
			auto found = path_in_pack.find(folder_name);
			if (found != std::string::npos) {
				path_in_pack = path_in_pack.substr(found);
			}

			if (ignore(path_in_pack)) {
				std::cout << FormatString("excluded: %s\n", file_path);
				continue;
			}

			fba_header h;
			h.index = index++;
			h.path = path_in_pack;
			std::cout << FormatString("packing : %s\n", path_in_pack.c_str());
			h.modified_time = FileSystem::GetLastModified(file_path);
			std::vector<char> deflated_data;
			std::ifstream source(file_path, std::fstream::binary);
			source.seekg(0, source.end);
			h.original_size = (unsigned int)source.tellg();
			original_size += h.original_size;
			source.seekg(0, source.beg);
			std::vector<char> source_data;
			source_data.resize(h.original_size);
			source.read(&source_data[0], h.original_size);
			auto ret = compress(deflated_data, source_data);
			if (ret != Z_OK) {
				std::cerr << "Compression failed.\n";
				return PFR_ERROR;
			}
#if FB_DATA_ENCRYPT
			if (!password.empty()) {
				encrypt_data(deflated_data, password);
			}
#endif
			h.deflated_size = deflated_data.size();
			compressed_size += h.deflated_size;


			h.start_pos = (unsigned)result_file.tellp();
#if FB_DATA_ENCRYPT
			write_encrypted_data(result_file, deflated_data, password, h);
#else
			result_file.write(&deflated_data[0], deflated_data.size());
#endif			
			headers.push_back(h);
		}
		auto data_end_pos = (unsigned)result_file.tellp();
		oar & headers;
		oar & data_end_pos;
		poar.reset();
		result_file.close();

		std::cout << FormatString("Results for folder(%s): total files(%u), original size(%u), compressed size(%u), compression ratio(%.1f%%)\n",
			folder_name.c_str(), headers.size(), original_size, compressed_size, (1.0f - (float)compressed_size / (float)original_size) * 100.f);
		out_original_size = original_size;
		out_compressed_size = compressed_size;

		if (perform_validation) {
			if (verify_fba_to_folder(output_file_path, data_folder, password, verify_bidirectional)) {
				return PFR_SUCCESS;
			}
			else {
				return PFR_ERROR;
			}
		}
		return PFR_SUCCESS;
	}

	bool parse_fba_headers(const char* path, pack_datum& data) {
		if (!ValidCString(path)) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}
		std::ifstream stream(path, std::fstream::binary);
		if (!stream) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open the file(%s)", path).c_str());
			return false;
		}
		boost::archive::binary_iarchive ar(stream);
		data.pack_path = path;
		fba_file_header file_header;
		ar & file_header;
		if (!file_header.is_valid_fba_type()) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Invalid fba file(%s)", path).c_str());
			return false;
		}
		data.version = file_header.version;
		data.resource_version = file_header.resource_version;
		data.passwd_hash = file_header.passwd_hash;

		stream.seekg(-4, stream.end);
		unsigned header_start;
		ar & header_start;
		stream.seekg(header_start);
		ar & data.headers;
		unsigned index = 0;
		for (auto& h : data.headers) {
			h.index = index;
			data.name_index[h.path] = index;
#if FB_FBA_IGNORE_CASE
			data.namelower_index[ToLowerCase(h.path.c_str())] = index;
#endif
			++index;
		}

		return true;
	}

	ByteArrayPtr parse_fba_data(const char* fba_path, const fba_header& h, const std::string& password) {
		if (!ValidCString(fba_path)) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}
		std::ifstream stream(fba_path, std::fstream::binary);
		if (!stream) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open the file(%s)", fba_path).c_str());
			return false;
		}
		return parse_fba_data(stream, h, password);
	}

	ByteArrayPtr parse_fba_data(std::istream& stream, const fba_header& h, const std::string& password) {		
		std::vector<char> compressed_data;
#if FB_DATA_ENCRYPT				
		read_encrypted_data(stream, compressed_data, password, h);
		decrypt_data(compressed_data, password);
#else
		stream.seekg(h.start_pos);
		compressed_data.resize(h.deflated_size);
		stream.read(&compressed_data[0], h.deflated_size);
#endif
		if (h.original_size > 0) {
			ByteArrayPtr data = std::make_shared<ByteArray>();
			data->resize(h.original_size);
			auto error = uncompress(*data, compressed_data);
			if (!error)
				return data;
			else {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"uncompress for (%s) failed.", h.path.c_str()).c_str());
				return nullptr;
			}
			return nullptr;
		}
		else {
			return std::make_shared<ByteArray>();
		}
		
	}


	enum file_patch_type {
		patch_no_need_to,
		patch_update,
		patch_new,
		patch_delete,
	};

	struct patch_header {
		std::string filepath;
		file_patch_type patch_type;
	};

	patch_header create_patch_header(const char* source_path, const char* path_in_pack, std::istream& source_stream, fba_headers& headers, 
		std::ifstream& current_fba, const std::string& password, bool date_only) {
		fba_header* found_header = 0;
		for (auto& h : headers) {
			if (strcmp(h.path.c_str(), path_in_pack) == 0) {
				found_header = &h;
				break;
			}
		}
		patch_header pheader;
		pheader.filepath = path_in_pack;
		if (!found_header) {
			pheader.patch_type = patch_new;
			std::cout << "new: " << pheader.filepath << std::endl;
			std::cerr << "new: " << pheader.filepath << std::endl;
			return pheader;
		}
		source_stream.seekg(0, std::fstream::end);
		unsigned source_size = (unsigned)source_stream.tellg();
		source_stream.seekg(0);
		if (found_header->original_size != source_size) {
			pheader.patch_type = patch_update;
			std::cout << "updated: " << pheader.filepath << std::endl;
			std::cerr << "updated: " << pheader.filepath << std::endl;
			return pheader;
		}
		auto source_modified_time = FileSystem::GetLastModified(source_path);
		if (found_header->modified_time < source_modified_time) {
			pheader.patch_type = patch_update;
			std::cout << "updated: " << pheader.filepath << std::endl;
			std::cerr << "updated: " << pheader.filepath << std::endl;
			return pheader;
		}

		if (!date_only) {
			std::vector<char> source_data(source_size);
			source_stream.read(&source_data[0], source_size);
			auto packed_data = parse_fba_data(current_fba, *found_header, password);
			if (memcmp(&source_data[0], &(*packed_data)[0], source_size) != 0) {
				pheader.patch_type = patch_update;
				return pheader;
			}
		}
		pheader.patch_type = patch_no_need_to;
		return pheader;
	}

	bool create_patch(const std::string& target_folder, const std::string& source_folder, const std::string& password,
		const std::string& ignore_file, bool date_only, bool perform_validation)
	{
		auto it = FileSystem::GetDirectoryIterator(target_folder.c_str(), false);
		if (!it) {
			std::cerr << "Cannot open the folder " << target_folder << std::endl;
			return false;
		}
		while (it->HasNext()) {
			bool is_dir;
			auto path = it->GetNextFilePath(&is_dir);
			if (is_dir)
				continue;
			if (FileSystem::HasExtension(path, FB_FBA_EXT)) {
				auto source_folder_path = FileSystem::ConcatPath(source_folder.c_str(), 
					FileSystem::GetName(path).c_str());
				if (!create_patch_for_fba(path, source_folder_path, password, ignore_file, date_only, perform_validation)) {
					std::cerr << "Failed to create a patch for " << path << std::endl;
					return false;
				}
			}
		}
		return true;
	}

	bool create_patch_for_fba(const std::string& fba_file_path, const std::string& source_folder_path, const std::string& password,
		const std::string& ignore_file, bool date_only, bool perform_validation)
	{			
		std::ifstream current_fba(fba_file_path, std::fstream::binary);
		if (!current_fba) {
			std::cerr << "Original .fba file is not found at (" << fba_file_path << ")\n";
			return false;
		}
		boost::archive::binary_iarchive ar(current_fba);
		fba_file_header file_header;
		ar & file_header;
		if (!file_header.is_valid_fba_type()) {
			std::cerr << "Original .fba file(" << fba_file_path << ") << has wrong format.\n";
			return false;
		}

		if (!file_header.is_valid_password(password)) {
			std::cerr << "Invalid password\n";
			return false;
		}

		current_fba.seekg(-4, current_fba.end);
		unsigned data_end_pos;
		ar & data_end_pos;
		current_fba.seekg(data_end_pos);

		fba_headers headers;
		ar & headers;

		unsigned index = 0;
		for (auto& h : headers) {
			h.index = index++;
		}

		// Do not need to know.
		// Just leave it for let one know the format.
		//fba_empty_spaces empty_spaces;
		//ar & empty_spaces;

		ignores.clear();
		if (!ignore_file.empty()) {
			std::ifstream file(ignore_file);
			if (file) {
				while (file) {
					std::string item;
					file >> item;
					if (!item.empty())
						ignores.insert(item);
				}
			}
		}
		auto fba_name = FileSystem::GetName(fba_file_path.c_str());
		auto source_parent = FileSystem::GetParentPath(source_folder_path.c_str());
		std::vector<patch_header> to_patch;
		auto it = FileSystem::GetDirectoryIterator(source_folder_path.c_str(), true);
		while (it && it->HasNext()) {
			bool is_dir;
			auto filepath = it->GetNextFilePath(&is_dir);
			if (is_dir)
				continue;
			if (FileSystem::HasExtension(filepath, ".fba") || ignore(filepath)) {
				std::cout << FormatString("excluded: %s\n", filepath);
				continue;
			}

			std::ifstream source_stream(filepath, std::fstream::binary);
			if (!source_stream) {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open the file(%s)", filepath).c_str());
				return false;
			}
			std::string filepath_in_header = filepath;
			auto found = filepath_in_header.find(fba_name);
			if (!found) {
				std::cerr << "Invalid folder path.\n";
				return false;
			}
			filepath_in_header = filepath_in_header.substr(found);	

			auto pheader = create_patch_header(filepath, filepath_in_header.c_str(), source_stream, headers, current_fba, password, date_only);
			if (pheader.patch_type != patch_no_need_to) {
				to_patch.push_back(pheader);
			}
		}

		// find deleted
		for (auto& h : headers) {
			auto source_path = FileSystem::ConcatPath(source_parent.c_str(), h.path.c_str());
			if (!FileSystem::Exists(source_path.c_str()) || ignore(source_path)) {
				to_patch.push_back(patch_header{ h.path, patch_delete });
			}
		}
		auto parent_folder = FileSystem::GetParentPath(fba_file_path.c_str());
		std::string patch_filename = FormatString("%s/%s_patch_%u%s", parent_folder.c_str(),
			FileSystem::GetName(fba_file_path.c_str()).c_str(), file_header.resource_version + 1, FB_FBAP_EXT);
		FileSystem::BackupFile(patch_filename.c_str(), 5, "patch_backup");
		BOOST_SCOPE_EXIT(void) {
			FileSystem::RemoveAll("patch_backup");
		}BOOST_SCOPE_EXIT_END;

		std::ofstream patch_stream(patch_filename.c_str(), std::ios::binary);
		boost::archive::binary_oarchive oar(patch_stream);
		fba_file_header patch_file_header =
			{ s_fba_patch_type[0], s_fba_patch_type[1], fba_version, file_header.resource_version+1,
			password.empty() ? 0 : murmur3_32(password.c_str(), password.size()) };

		oar & patch_file_header;

		std::vector<std::string> deleted_files;
		std::vector<fba_header> new_neaders;
		index = 0;
		for (auto& ph : to_patch) {
			if (ph.patch_type == patch_delete) {
				deleted_files.push_back(ph.filepath);
				continue;
			}
			fba_header h;
			h.path = ph.filepath;
			h.index = index++;
			auto actual_path = FileSystem::ConcatPath(source_parent.c_str(), h.path.c_str());
			h.modified_time = FileSystem::GetLastModified(actual_path.c_str());
			std::ifstream source_file(actual_path, std::fstream::binary);
			source_file.exceptions(std::ios::badbit | std::ios::failbit);
			std::vector<char> source_data;
			try {
				source_file.seekg(0, std::fstream::end);
				unsigned source_size = (unsigned)source_file.tellg();
				source_file.seekg(0);
				h.original_size = (unsigned)source_size;
				source_data.resize(source_size);
				source_file.read(&source_data[0], source_size);
			}
			catch (std::ios_base::failure& e) {
				if (e.code() == std::make_error_condition(std::io_errc::stream)) {
					std::cerr << "Stream error!\n";
					std::cerr << e.what() << std::endl;
				}
				else {
					std::cerr << "Unknown failure opening file.\n";
				}
				return false;
			}

			std::vector<char> deflated_data;
			auto ret = compress(deflated_data, source_data);
			if (ret != Z_OK) {
				std::cerr << "Compression failed.\n";
				return false;
			}
#if FB_DATA_ENCRYPT
			if (!password.empty()) {
				encrypt_data(deflated_data, password);
			}
#endif
			h.deflated_size = deflated_data.size();
			h.start_pos = (unsigned)patch_stream.tellp();
#if FB_DATA_ENCRYPT
			write_encrypted_data(patch_stream, deflated_data, password, h);
#else
			result_file.write(&deflated_data[0], deflated_data.size());
#endif			
			new_neaders.push_back(h);
		}
		unsigned patch_data_end_pos = (unsigned)patch_stream.tellp();
		oar & new_neaders;
		oar & deleted_files;
		oar & patch_data_end_pos;
		return true;
	}

	bool write_header_and_update_filesize(std::fstream& stream, const char* stream_path, unsigned data_end_pos, const fba_headers& headers) {
		{
			stream.flush();
			stream.seekg(0);
			stream.seekp(0);

			boost::archive::binary_oarchive ar(stream);
			stream.seekp(data_end_pos);
			ar & headers;
			ar & data_end_pos;
		}
		auto actual_size = stream.tellp();
		stream.seekp(0, std::fstream::end);
		auto cur_size = stream.tellp();
		stream.close();
		if (actual_size < cur_size)
			FileSystem::ResizeFile(stream_path, (unsigned)actual_size);
		return true;
	}

	bool remove_file(std::fstream& stream, fba_headers& headers, const fba_header& header,
		const std::string& password, unsigned& data_end_pos)
	{
		if (!stream) {
			std::cerr << "stream is not open\n";
			return false;
		}
		unsigned removed_index = header.index;
		auto next_write_pos = header.start_pos;
		for (unsigned i = removed_index + 1; i < headers.size(); ++i) {
			auto& moving_header = headers[i];
			std::vector<char> data;
#if FB_DATA_ENCRYPT	
			read_encrypted_data(stream, data, password, moving_header);
#else
			stream.seekg(moving_header.start_pos);
			data.resize(moving_header.deflated_size);
			stream.read(&data[0], data.size());
#endif
			/*decrypt_data(data, password);
			std::vector<char> uncompress_data(moving_header.original_size);
			if (auto err = uncompress(uncompress_data, data)) {
				std::cerr << "Cannot uncompress data.\n";
				return false;
			}
			encrypt_data(data, password);*/
			stream.seekp(next_write_pos);
			moving_header.start_pos = next_write_pos;
			--moving_header.index;
#if FB_DATA_ENCRYPT				
			write_encrypted_data(stream, data, password, moving_header);
#else
			stream.write(&data[0], data.size());
#endif
			next_write_pos = (unsigned)stream.tellp();
		}
		headers.erase(headers.begin() + removed_index);
		data_end_pos = next_write_pos;
		return true;
	}

	bool remove_file(const char* fba_file_path, const char* removing_file, const std::string& password) {
		if (!ValidCString(fba_file_path) || !ValidCString(removing_file)) {
			std::cerr << "Invalid args.\n";
			return false;
		}		
		std::fstream stream(fba_file_path, std::fstream::in | std::fstream::out | std::fstream::binary);
		boost::archive::binary_iarchive ar(stream);
		fba_file_header file_header;
		ar & file_header;
		if (file_header.is_valid_fba_type()) {
			std::cerr << "Invalid file type.\n";
			return false;
		}
		if (!file_header.is_valid_password(password)) {
			std::cerr << "Invalid password.\n";
			return false;
		}


		unsigned data_end_pos;
		stream.seekg(-4, std::fstream::end);
		ar & data_end_pos;
		stream.seekg(data_end_pos);
		fba_headers headers;
		ar & headers;
		unsigned index = 0;
		for (auto& h : headers) {
			h.index = index++;
		}

		fba_header* found_header = 0;
		for (auto& header : headers) {
			if (strcmp(header.path.c_str(), removing_file) == 0) {
				found_header = &header;
				break;
			}
		}
		if (!found_header) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("File(%s) does not exist in the pack(%s)", removing_file, fba_file_path).c_str());
			return false;
		}

		auto removed = remove_file(stream, headers, *found_header, password, data_end_pos);

		if (removed) {
			write_header_and_update_filesize(stream, fba_file_path, data_end_pos, headers);
		}
		return removed;
	}

	bool add_file(std::fstream& dest_stream, fba_headers& dest_headers, std::ifstream& source_stream, const fba_header& source_header,
		const std::string& password, unsigned& dest_data_end_pos)
	{
		// make sure file not exists - you should delete it first using remove_file()
		for (auto& dh : dest_headers) {
			if (dh.path == source_header.path) {
				std::cerr << "file " << dh.path << " exists. You should delete it with remove_file() first.\n";
				return false;
			}
		}

		dest_headers.push_back(source_header);
		auto& h = dest_headers.back();
		h.index = dest_headers.size() - 1;
		h.start_pos = dest_data_end_pos;

		std::vector<char> adding_data;
#if FB_DATA_ENCRYPT	
		read_encrypted_data(source_stream, adding_data, password, source_header);
#else
		source_stream.seekg(source_header.start_pos);
		adding_data.resize(source_header.deflated_size);
		source_stream.read(&adding_data[0], adding_data.size());
#endif


		dest_stream.seekp(dest_data_end_pos);
#if FB_DATA_ENCRYPT	
		write_encrypted_data(dest_stream, adding_data, password, h);
#else
		dest_stream.write(&adding_data[0], adding_data.size());
#endif

		dest_data_end_pos = (unsigned)dest_stream.tellp();
		return true;
	}

	bool apply_patch(const std::string& target_folder, const std::string& source_folder, const std::string& password, bool validation)
	{
		auto dir_it = FileSystem::GetDirectoryIterator(target_folder.c_str(), false);
		if (!dir_it) {
			std::cerr << "Cannot open directory ./\n";
			return false;
		}
		// dest, src
		unsigned numFbas = 0;
		std::vector<std::pair<std::string, std::string>> patches;
		while (dir_it->HasNext()) {
			bool is_dir;
			const char* path = dir_it->GetNextFilePath(&is_dir);
			if (is_dir)
				continue;
			if (FileSystem::HasExtension(path, "fba")) {
				fba_file_header file_header, file_header_p;
				{
					std::ifstream stream(path, std::ios::binary);
					if (!stream) {
						std::cerr << "Cannot open " << path << std::endl;
						return false;
					}
					boost::archive::binary_iarchive ar(stream);
					ar & file_header;
				}
				if (!file_header.is_valid_fba_type()) {
					continue;
				}
				if (!file_header.is_valid_password(password)) {
					std::cerr << "Invalid password!\n";
					return false;
				}
				numFbas++;
				auto patch_path = FileSystem::ConcatPath(source_folder.c_str(),
					FormatString("%s_patch_%u%s", FileSystem::GetName(path).c_str(), file_header.resource_version + 1, FB_FBAP_EXT).c_str());
				{
					std::ifstream stream(patch_path, std::ios::binary);
					if (!stream) {
						std::cerr << "Cannot open " << patch_path << std::endl;
						continue;
					}
					boost::archive::binary_iarchive ar(stream);
					ar & file_header_p;
					if (!file_header_p.is_valid_fba_patch_type())
						continue;
					if (!file_header_p.is_valid_password(password)) {
						std::cerr << "Invalid password!\n";
						return false;
					}
				}
				patches.push_back({ path, patch_path });				
			}
		}

		if (numFbas != patches.size()) {
			std::cerr << "Invalid number of patch files.\n";
			return false;
		}
		auto temp_dir = FileSystem::GetTempDir();
		temp_dir += "patch/";
		std::vector< std::pair<std::string, std::string> > backups;
		for (auto& it : patches) {
			auto path_to_backup = FileSystem::BackupFile(it.first.c_str(), 1, temp_dir.c_str());
			if (!path_to_backup.empty())
				backups.push_back({ it.first,  path_to_backup });
		}
		bool applied = false;
		BOOST_SCOPE_EXIT(&applied, &backups, &temp_dir, &patches) {
			bool anyError = false;
			if (!applied) {				
				for (auto& it : backups) {
					auto error = FileSystem::CopyFile(it.second.c_str(), it.first.c_str(), true, false);
					if (error != 0) {
						anyError = true;
						std::cerr << "Could not recover the file " << it.first << " from " << it.second << ".\n";
						std::cerr << "Error code: " << error << "\n";
					}
				}
			}
			if (!anyError) {
				FileSystem::RemoveAllInside(temp_dir.c_str());
				for (auto& it : patches) {
					auto destfilePath = temp_dir;
					destfilePath += FileSystem::GetFileName(it.second.c_str());
					FileSystem::Rename(it.second.c_str(), destfilePath.c_str());
				}
			}
		}BOOST_SCOPE_EXIT_END;		

		for (auto& it : patches) {
			bool success = apply_patch_individual(it.first, it.second, password, false);
			if (!success) {
				std::cerr << "File to apply patch.\n";
				return false;
			}
		}
		applied = true;
		std::cout << "Patch completed.\n";
		std::cerr << "Patch completed.\n";
		return true;
	}

	bool apply_patch_individual(const std::string& dest_file, const std::string& source_file,
		const std::string& password, bool backup)
	{
		if (dest_file.empty() || source_file.empty()) {
			std::cerr << "Invalid arg.\n";
			return false;
		}	
		
		bool applied = false;
		std::string path_to_backup;
		if (backup) {
			path_to_backup = FileSystem::BackupFile(dest_file.c_str(), 1, "patch_backup");
		}
		BOOST_SCOPE_EXIT(&path_to_backup) {
			if (!path_to_backup.empty())
				FileSystem::Remove(path_to_backup.c_str());
		}BOOST_SCOPE_EXIT_END;

		std::fstream dest_stream(dest_file, std::fstream::in | std::fstream::out | std::fstream::binary);
		if (!dest_stream) {
			std::cerr << "Cannot open the file " << dest_file << ".\n";
			char buf[512];
			strerror_s(buf, errno);
			std::cerr << buf << std::endl;
			return false;
		}

		std::ifstream source_stream(source_file, std::fstream::binary);
		if (!source_stream) {
			std::cerr << "Cannot open the file " << source_file << ".\n";
			return false;
		}

		fba_file_header file_header;
		fba_headers dest_headers;
		unsigned pos_after_archive_header;
		unsigned dest_data_end_pos;
		{
			boost::archive::binary_iarchive destar(dest_stream);
			pos_after_archive_header = (unsigned)dest_stream.tellg();
			destar & file_header;
			if (!file_header.is_valid_fba_type()) {
				std::cerr << dest_file << " is not a fba pack.\n";
				return false;
			}
			if (!file_header.is_valid_password(password)) {
				std::cerr << "Invalid password.\n";
				return false;
			}
			dest_stream.seekg(-4, std::fstream::end);
			destar & dest_data_end_pos;
			dest_stream.seekg(dest_data_end_pos);
			destar & dest_headers;
			unsigned index = 0;
			for (auto& h : dest_headers) {
				h.index = index++;
			}
		}

		unsigned source_data_end_pos;
		fba_headers source_headers;
		std::vector<std::string> deleted_files;
		{
			boost::archive::binary_iarchive sourcear(source_stream);
			fba_file_header source_file_header;
			sourcear & source_file_header;
			if (!source_file_header.is_valid_fba_patch_type()) {
				std::cerr << dest_file << " is not a fbap file.\n";
				return false;
			}
			if (!source_file_header.is_valid_password(password)) {
				std::cerr << "Invalid password for the patch.\n";
				return false;
			}
			if (source_file_header.resource_version-1 != file_header.resource_version) {
				std::cerr << "Resource version mismatch. This fbap file is for resource version " << source_file_header.resource_version
					<< ". The dest pack file resource version is " << file_header.resource_version << ".\n";
				return false;
			}
			source_stream.seekg(-4, std::fstream::end);
			sourcear & source_data_end_pos;
			source_stream.seekg(source_data_end_pos);
			sourcear & source_headers;
			sourcear & deleted_files;

			unsigned index = 0;
			for (auto& h : source_headers) {
				h.index = index++;
			}
		}

		BOOST_SCOPE_EXIT(&applied, &dest_stream, &dest_file, &path_to_backup) {
			if (!applied) {
				if (!path_to_backup.empty()) {
					auto error = FileSystem::CopyFile(path_to_backup.c_str(), dest_file.c_str(), true, false);
					if (error != 0) {
						std::cerr << "Could not recover the file " << dest_file << " from " << path_to_backup << ".\n";
						std::cerr << "Error code: " << error << "\n";
					}
				}
			}
		}BOOST_SCOPE_EXIT_END;

		dest_stream.exceptions(std::ios::failbit | std::ios::badbit);
		try {
			// delete first
			for (auto it = deleted_files.rbegin(); it != deleted_files.rend(); ++it) {
				for (auto& h : dest_headers) {
					if (h.path == *it) {
						if (!remove_file(dest_stream, dest_headers, h, password, dest_data_end_pos)) {
							return false;
						}
						break;
					}
				}
			}

			for (auto& sh : source_headers) {
				for (auto& h : dest_headers) {
					if (h.path == sh.path) {
						if (!remove_file(dest_stream, dest_headers, h, password, dest_data_end_pos)) {
							return false;
						}
						break;
					}
				}
			}
			for (auto& sh : source_headers) {
				if (!add_file(dest_stream, dest_headers, source_stream, sh, password, dest_data_end_pos)) {
					return false;
				}
			}

			dest_stream.seekg(0);
			dest_stream.seekp(0);
			boost::archive::binary_oarchive destar(dest_stream);
			auto t1 = dest_stream.tellg();
			dest_stream.seekp(pos_after_archive_header);
			file_header.resource_version += 1;
			destar & file_header;
			if (!dest_stream) {
				std::cerr << "failed to write new version.\n";
				return false;
			}
			if (!write_header_and_update_filesize(dest_stream, dest_file.c_str(), dest_data_end_pos, dest_headers)) {
				return false;
			}
		}
		catch (std::ios_base::failure& e) {
			if (e.code() == std::make_error_condition(std::io_errc::stream)) {
				std::cerr << "Stream error!\n";
				std::cerr << e.what() << std::endl;
			}
			else {
				std::cerr << "Unknown failure opening file.\n";
			}
			return false;
		}

		if (!verify_fba_to_folder(dest_file, "", password, verify_uncompress)) {
			return false;
		}

		applied = true;
		return true;
	}


	bool verify_fba_to_folder(const std::string& fba_file_path, const std::string& folder_path,
		const std::string& password, fba_verify_type vtype) 
	{
		std::ifstream test_file(fba_file_path, std::fstream::binary);
		boost::archive::binary_iarchive ar(test_file);
		fba_file_header file_header;
		ar & file_header;

		if (!file_header.is_valid_fba_type()) {
			std::cerr << "Validation failed: Invalid fba type.\n";
			return false;
		}

		if (!file_header.is_valid_password(password)) {
			std::cerr << "Validation failed: Invalid password.";
			return false;
		}

		test_file.seekg(-4, test_file.end);
		unsigned data_end_pos2;
		ar & data_end_pos2;
		test_file.seekg(data_end_pos2);
		fba_headers headers2;
		ar & headers2;
		unsigned index = 0;
		for (auto& h : headers2) {
			h.index = index++;
		}
		std::string fba_name = FileSystem::GetName(fba_file_path.c_str());
		std::cout << "Validating " << fba_file_path << std::endl;
		auto appdata = FileSystem::GetTempDir();		
		std::string validation_folder = appdata + "validation/";
		FileSystem::RemoveAll(validation_folder.c_str());
		FileSystem::CreateDirectory(validation_folder.c_str());
		BOOST_SCOPE_EXIT(&validation_folder) {
			FileSystem::RemoveAll(validation_folder.c_str());
		} BOOST_SCOPE_EXIT_END;


		for (auto& h : headers2) {
			std::vector<char> compressed_data;
#if FB_DATA_ENCRYPT				
			read_encrypted_data(test_file, compressed_data, password, h);
			decrypt_data(compressed_data, password);
#else
			test_file.seekg(h.start_pos);
			compressed_data.resize(h.deflated_size);
			test_file.read(&compressed_data[0], compressed_data.size());
#endif
			std::vector<char> data;
			data.resize(h.original_size);
			if (h.original_size != 0) {
				auto error = uncompress(data, compressed_data);
				if (error) {
					std::cerr << "Validation failed: Uncompression failed.\n";
					return false;
				}
			}
			if (vtype == verify_uncompress)
				continue;
			std::string actual_data_path;
			auto found = folder_path.find(fba_name);
			if (found != std::string::npos) {
				actual_data_path = folder_path.substr(0, found);
			}
			actual_data_path += h.path;
			std::ifstream src_file(actual_data_path, std::fstream::binary);
			if (!src_file) {
				std::cerr << FormatString("Validation failed : This file(%s) is deleted one.\n", h.path.c_str());
				return false;
			}
			src_file.seekg(0, std::fstream::end);
			unsigned src_size = (unsigned)src_file.tellg();
			if (src_size != h.original_size) {
				std::cerr << FormatString("Validation failed : File(%s) size is different. in pack(%u), not in pack(%u)\n", 
					h.path, h.original_size, src_size);
				return false;
			}
			src_file.seekg(0);
			std::vector<char> src_data(src_size);
			src_file.read(&src_data[0], src_size);
			if (memcmp(&src_data[0], &data[0], src_size) != 0) {
				std::cerr << FormatString("Validation failed : File(%s) data is different.\n", h.path);
				return false;
			}
			if (vtype == verify_bidirectional) {
				auto validation_file_path = validation_folder + h.path;
				auto parent_path = FileSystem::GetParentPath(validation_file_path.c_str());
				FileSystem::CreateDirectory(parent_path.c_str());
				std::ofstream stream(validation_file_path, std::ofstream::binary);
				if (stream) {
					stream.write((char*)&data[0], data.size());
				}
				else {
					std::cerr << FormatString(
						"Validation failed : Cannot create a file(%s)\n", validation_file_path.c_str());
					return false;
				}
			}
		}

		if (vtype == verify_uncompress || vtype == verify_only_in_pack) {
			std::cout << "Validation done." << std::endl;
			return true;
		}

		auto srcIt = FileSystem::GetDirectoryIterator(folder_path.c_str(), true);
		while (srcIt && srcIt->HasNext()) {
			auto filepath = srcIt->GetNextFilePath();
			if (FileSystem::IsDirectory(filepath))
				continue;
			if (ignore(filepath))
				continue;
			std::ifstream src_file(filepath, std::fstream::binary);
			if (!src_file) {
				std::cerr << FormatString(
					"Validation failed : cannot open the original file(%s) not exists\n", filepath);
				return false;
			}
			std::string strfilepath(filepath);
			auto found = strfilepath.find(fba_name);
			std::string path_in_pack;
			if (found != std::string::npos) {
				path_in_pack = strfilepath.substr(found);
			}
			
			auto check_filepath = validation_folder + path_in_pack;
			std::ifstream check_file(check_filepath, std::fstream::binary);
			if (!check_file) {
				std::cerr << FormatString(
					"Validation failed : checking file(%s) not exists\n", check_filepath.c_str());
				return false;
			}
			src_file.seekg(0, src_file.end);
			auto src_size = src_file.tellg();
			src_file.seekg(0, src_file.beg);

			check_file.seekg(0, check_file.end);
			auto check_size = check_file.tellg();
			check_file.seekg(0, check_file.beg);
			if (src_size != check_size) {
				std::cerr << FormatString(
					"Validation failed : original size(%u) is different from the result file size(%u)\n", src_size, check_size);
				return false;
			}
			std::vector<char> src_data((unsigned)src_size);
			src_file.read(&src_data[0], src_size);
			std::vector<char> check_data((unsigned)check_size);
			check_file.read(&check_data[0], check_size);
			int i = 0;
			for (auto c : src_data) {
				if (c != check_data[i++]) {
					std::cerr << FormatString(
						"Validation failed : data is invalid.\n", src_size, check_size);
					return false;
				}
			}
		}

		std::cout << "Validation done." << std::endl;
		return true;
	}

	bool extract_fba(const char* fba_file, const std::string& to_, const std::string& password) {
		if (!ValidCString(fba_file)) {
			std::cerr << "Invalid arg.\n";
			return false;
		}

		auto to = FileSystem::AddEndingSlashIfNot(to_.c_str());		
		std::ifstream fba(fba_file, std::ios::binary);
		if (!fba) {
			std::cerr << FormatString("Cannot open %s.\n", fba_file);
			return false;
		}

		boost::archive::binary_iarchive ar(fba);
		fba_file_header file_header;
		ar & file_header;
		if (!file_header.is_valid_fba_type()) {
			std::cerr << FormatString("%s is not fba file.\n", fba_file);
			return false;
		}
		if (!file_header.is_valid_password(password)) {
			std::cerr << FormatString("Invalid password for %s.\n", fba_file);
			return false;
		}
		fba.seekg(-4, std::ios::end);
		unsigned data_end_pos;
		ar & data_end_pos;
		
		fba.seekg(data_end_pos);
		fba_headers headers;
		ar & headers;
		unsigned index = 0;
		for (auto& h : headers) {
			h.index = index++;
		}

		for (auto& h : headers) {
			std::vector<char> compressed_data;
#if FB_DATA_ENCRYPT				
			read_encrypted_data(fba, compressed_data, password, h);
			decrypt_data(compressed_data, password);
#else
			test_file.seekg(h.start_pos);
			compressed_data.resize(h.deflated_size);
			test_file.read(&compressed_data[0], compressed_data.size());
#endif
			std::vector<char> data;
			if (h.original_size > 0) {
				data.resize(h.original_size);
				auto error = uncompress(data, compressed_data);
				if (error) {
					std::cerr << "Validation failed: Uncompression failed.\n";
					return false;
				}
			}
			
			std::string dest_path = to + h.path;
			auto parent_path = FileSystem::GetParentPath(dest_path.c_str());
			FileSystem::CreateDirectory(parent_path.c_str());
			std::ofstream stream(dest_path, std::ofstream::binary);
			if (stream) {
				stream.write((char*)&data[0], data.size());
			}
			else {
				std::cerr << FormatString("Cannot create a file(%s)\n", dest_path.c_str());
				return false;
			}
		}
		return true;
	}
}

BOOST_CLASS_IMPLEMENTATION(fb::fba_file_header, boost::serialization::primitive_type);