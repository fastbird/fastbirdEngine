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
#include "stdafx.h"
#include "FileSystem.h"
#include "DirectoryIterator.h"
#include "File.h"
#include "FBCommonHeaders/SpinLock.h"
#include "FBCommonHeaders/RecursiveSpinLock.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBDataPackLib/fba.h"
#include "FBSerializationLib/Serialization.h"
#include "boost/archive/binary_iarchive.hpp"
using namespace fb;

static bool gLogginStarted = false;
static boost::filesystem::path gWorkingPath;
std::string gApplicationName;
static std::unordered_map<std::string, std::string> sResourceFolders;
static std::unordered_map<std::string, std::string> sResourceFoldersLower;
static std::unordered_map<std::string, pack_datum> g_fbas;
static std::unordered_map<std::string, std::pair<const fba_header*, const pack_datum*> > g_path_header_cache;
#define FBRExt "fbr"
static RecursiveSpinLock<true, true> sGuard;
typedef std::chrono::time_point<std::chrono::system_clock> SystemTimePoint;

std::mutex pack_mutex;
static std::string g_game_home;
namespace fb {
	void OnInit() {
		g_game_home = boost::filesystem::current_path().generic_string();
	}
}

void FileSystem::StartLoggingIfNot(){
	if (gLogginStarted)
		return;
	auto filepath = "_FBFileSystem.log";
	FileSystem::BackupFile(filepath, 5, "Backup_Log");
	Logger::Init(filepath);
	gLogginStarted = true;
}

void FileSystem::StopLogging(){
	if (!gLogginStarted)
		return;

	Logger::Release();
}

unsigned FileSystem::parse_fba(const char* path) {
	unsigned num = 0;
	auto it = GetDirectoryIterator(path, false);
	while (it->HasNext()) {
		std::string fba_path = it->GetNextFilePath();
		if (IsDirectory(fba_path.c_str()))
			continue;
		if (HasExtension(fba_path.c_str(), ".fba")) {			
			if (StartsWith(fba_path, "./")) {
				fba_path = fba_path.substr(2);
			}
			pack_datum data;
			if (parse_fba_headers(fba_path.c_str(), data)) {
				auto key = ReplaceExtension(fba_path.c_str(), "");				
				// key = Data/actors for children folders
				// key = D:/Projects/fastbird-engine/EssentialEngineData for resource folders
				++num;
				g_fbas[key] = data; 
			}
			else {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Parsing fba(%s) is failed.", fba_path).c_str());
			}
		}
	}
	return num;
}

// path_in_pack : actors/myactor.lua
const fba_header* get_fba_header(pack_datum& pack, const char* path_in_pack) {
	auto it = pack.name_index.find(path_in_pack);
	if (it != pack.name_index.end()) {
		auto& h = pack.headers[it->second];		
		return &h;
	}

#if FB_FBA_IGNORE_CASE
	// Todo : after removing all case differences and comment below out.
	auto it2 = pack.namelower_index.find(ToLowerCase(path_in_pack));
	if (it2 != pack.namelower_index.end()) {
		auto originalIt = pack.name_index.begin();
		for (; originalIt != pack.name_index.end(); originalIt++) {
			if (!StringCompareNoCase(originalIt->first, it2->first))
				break;
		}
		if (originalIt == pack.name_index.end()) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Cannot find original file for (%s)", it2->first.c_str()).c_str());
		}
		else {
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString(
				"(Info) Ignoring case: Requested File(%s) is detected by ignoring case. Chage the request name to (%s)",
				path_in_pack, originalIt->first.c_str()).c_str());
		}
		auto& h = pack.headers[it2->second];
		std::lock_guard<std::mutex> l(pack_mutex);
		pack.name_index[path_in_pack] = h.index;
		return &h;		
	}	
#endif

	return nullptr;
}

const fba_header* get_fba_header(const char* path, std::string& fba_path) {
	auto it = g_path_header_cache.find(path);
	if (it != g_path_header_cache.end()) {
		fba_path = it->second.second->pack_path;
		return it->second.first;
	}

	for (auto it = g_fbas.begin(); it != g_fbas.end(); ++it) {
		// it->first: Data/actors
		// path: Data/actors/myactor.lua

		if (StartsWith(path, it->first.c_str(), false)) {
			auto parent_path = FileSystem::GetParentPath(it->first.c_str()); // Data
			auto path_in_pack = path + (parent_path.empty() ? 0 : (parent_path.size() + 1));
			auto header = get_fba_header(it->second, path_in_pack);
			fba_path = it->second.pack_path;
			std::lock_guard<std::mutex> l(pack_mutex);
			g_path_header_cache[path] = { header, &it->second };
			return header;
		}
#if FB_FBA_IGNORE_CASE
		if (StartsWith(path, it->first.c_str(), true)) {
			auto parent_path = FileSystem::GetParentPath(it->first.c_str()); // Data
			auto path_in_pack = path + (parent_path.empty() ? 0 : (parent_path.size() + 1));			
			auto path_in_pack_correct_case = std::string(path);
			path_in_pack_correct_case.replace(path_in_pack_correct_case.begin(), path_in_pack_correct_case.begin() + it->first.length(),
				it->first.begin(), it->first.end());

			auto header = get_fba_header(it->second, path_in_pack);
			fba_path = it->second.pack_path;
			std::lock_guard<std::mutex> l(pack_mutex);
			g_path_header_cache[path] = { header, &it->second };
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString(
				"(Info) Ignoring case: the requested path(%s) should be (%s)", path, path_in_pack_correct_case.c_str()).c_str());
			return header;
		}
#endif
	}
	return 0;
}
static std::string g_fba_password;
void FileSystem::set_fba_password(const char* password) {
	if (!ValidCString(password)) {
		g_fba_password.clear();
		return;
	}
	g_fba_password = password;
}

struct FbaFileDataCache {
	ByteArrayPtr data;
	SystemTimePoint last_access;
};
std::unordered_map<std::string, FbaFileDataCache> g_file_data_cache;
SystemTimePoint last_fba_removal = std::chrono::system_clock::now();
ByteArrayPtr FileSystem::get_fba_file_data(const char* path) {
	using namespace std::chrono_literals;
	auto now = std::chrono::system_clock::now();
	if (now - last_fba_removal > 10s) {
		last_fba_removal = now;
		std::lock_guard<std::mutex> l(pack_mutex);
		for (auto it = g_file_data_cache.begin(); it != g_file_data_cache.end(); ) {
			auto curIt = it++;
			if (now - curIt->second.last_access > 10s) {				
				g_file_data_cache.erase(curIt);
			}
		}
	}

	{
		std::lock_guard<std::mutex> l(pack_mutex);
		auto it = g_file_data_cache.find(path);
		if (it != g_file_data_cache.end()) {
			it->second.last_access = now;
			return it->second.data;
		}
	}
	std::string fba_path;	
	auto header = get_fba_header(path, fba_path);
	if (header) {
		auto data = parse_fba_data(fba_path.c_str(), *header, g_fba_password);
		std::lock_guard<std::mutex> l(pack_mutex);
		g_file_data_cache[path] = FbaFileDataCache{ data, now };
		return data;
	}

	return nullptr;
}

pack_datum* get_pack_datum_containing(const char* path) {
	for (auto& it : g_fbas) {
		if (StartsWith(path, it.first.c_str())) {
			return &it.second;
		}
	}
	return nullptr;
}

bool FileSystem::ExistsInFba(const char* path) {	
	std::string fba_path;
	auto header = get_fba_header(path, fba_path);
	return header != nullptr;
}
bool FileSystem::Exists(const char* path){	
	boost::system::error_code err;
	bool exists = boost::filesystem::exists(path, err);
	if (exists)
		return true;
	std::string pack_path;
	return ExistsInFba(path)!=0;
}

bool FileSystem::ResourceExists(const char* path, std::string* outResoucePath) {	
	auto ex = Exists(path);
	if (ex) {
		if (outResoucePath)
			*outResoucePath = path;
		return ex;
	}
	else{
		boost::system::error_code err;
		auto resourcePath = GetResourcePath(path);		
		ex = Exists(resourcePath.c_str());			
		if (ex) {
			if (outResoucePath)
				*outResoucePath = resourcePath;
			return ex;
		}
	}
	return false;
}

bool FileSystem::IsDirectory(const char* path){
	auto dir =  boost::filesystem::is_directory(path);
	if (dir)
		return true;
	else if (strrchr(path, '.') == 0) {
		// check resource path		
		return IsResourceDirectory(path);
	}
	return false;
}

bool FileSystem::IsResourceDirectory(const char* path) {
	auto p = strrchr(path, '/');
	if (!p) {
		for (auto it : sResourceFolders) {
			auto pos = it.second.find(path);
			if (pos != std::string::npos)
				return true;
		}
	}
	else {
		for (auto it : sResourceFolders) {
			if (StartsWith(path, it.first.c_str(), true)) {
				std::string newPath = it.second;
				newPath += boost::filesystem::path(path).generic_string().substr(it.first.size());
				return boost::filesystem::is_directory(newPath.c_str());
			}
		}
	}	
	return false;
}

size_t FileSystem::GetFileSize(const char* path) {
	if (Exists(path))
		return (unsigned)boost::filesystem::file_size(path);
	else {
		auto resourcePath = GetResourcePath(path);
		return (unsigned)boost::filesystem::file_size(resourcePath);
	}
}

int FileSystem::Rename(const char* path, const char* newpath){
	if (!Exists(path)){
		return RENAME_NO_SOURCE;
	}
	
	if (!CheckSecurity(path) || !CheckSecurity(newpath))
		return SECURITY_NOT_OK;
	try{
		boost::filesystem::rename(path, newpath);
	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	
	return FB_NO_ERROR;
}

int FileSystem::CopyFile(const char* src, const char* dest, bool overwrite, bool supressErrorMsg){
	if (!Exists(src)){
		return COPYFILE_NO_SOURCE;
	}
	if (!overwrite && Exists(dest)){
		return COPYFILE_DEST_ALREADY_EXISTS;
	}
	if (overwrite){
		try{
			boost::filesystem::copy_file(src, dest, boost::filesystem::copy_option::overwrite_if_exists);
		}
		catch (boost::filesystem::filesystem_error& err){
			if (!supressErrorMsg)
				Logger::Log(FB_ERROR_LOG_ARG, err.what());
			return COPYFILE_ERROR;
		}
	}
	else{
		try{
			boost::filesystem::copy_file(src, dest);
		}
		catch (boost::filesystem::filesystem_error& err){
			if (!supressErrorMsg)
				Logger::Log(FB_ERROR_LOG_ARG, err.what());
			return COPYFILE_ERROR;
		}
	}

	return FB_NO_ERROR;
}

void FileSystem::ResizeFile(const char* path, unsigned new_size) {
	if (!CheckSecurity(path)) {
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot resize the file(%s) for the sake of security.", path).c_str());
		return;
	}
	try {
		boost::filesystem::resize_file(path, new_size);
	}
	catch (const boost::filesystem::filesystem_error& err) {
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Cannot resize the file(%s).", path).c_str());
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
}

bool FileSystem::Remove(const char* path){
	bool ret = true;
	try{
		ret = boost::filesystem::remove(path);
	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}	
	return ret;
}

size_t FileSystem::RemoveAll(const char* path) {	
	size_t ret = true;
	try {
		ret = (size_t)boost::filesystem::remove_all(path);
	}
	catch (boost::filesystem::filesystem_error& err) {
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return ret;
}

size_t FileSystem::RemoveAllInside(const char* path) {
	size_t num = 0;
	if (IsDirectory(path)) {
		namespace fs = boost::filesystem;
		fs::path path_to_remove(path);
		for (fs::directory_iterator end_dir_it, it(path_to_remove); it != end_dir_it; ++it) {
			num += (size_t)fs::remove_all(it->path());
		}
	}
	return num;
}

std::string FileSystem::ReplaceExtension(const char* path, const char* ext){
	boost::filesystem::path boostPath(path);
	boostPath.replace_extension(ext);
	return boostPath.generic_string();
}

std::string FileSystem::ReplaceFilename(const char* path, const char* newFilename){
	boost::filesystem::path boostPath(path);
	boostPath = boostPath.remove_filename();
	boostPath.append(newFilename);
	return boostPath.generic_string();
}

const char* FileSystem::GetExtension(const char* path){
	size_t len = strlen(path);
	if (len == 0)
		return "";
	while (--len)
	{
		if (path[len] == '.')
		{
			return &path[len];
		}
	}
	return "";
}

const char* FileSystem::GetExtensionWithOutDot(const char* path){
	size_t len = strlen(path);
	int count = 0;
	while (--len)
	{
		if (path[len] == '.')
		{
			if (count == 0)
				return "";
			else
				return &path[len+1];
		}
		++count;
	}
	return "";
}

bool FileSystem::HasExtension(const char* filepath, const char* extension) {
	if (!ValidCString(extension) || !ValidCString(filepath))
		return false;
	if (extension[0] == '.') {
		return _stricmp(GetExtension(filepath), extension) == 0;
	}
	else {
		return _stricmp(GetExtensionWithOutDot(filepath), extension) == 0;
	}
}

std::string FileSystem::GetFileName(const char* path){
	boost::filesystem::path filepath(path);
	return filepath.filename().generic_string();
}

std::string FileSystem::GetName(const char* path){
	boost::filesystem::path filepath(path);
	return filepath.stem().generic_string();
}

std::string FileSystem::GetParentPath(const char* path){
	boost::filesystem::path filepath(path);
	return filepath.parent_path().generic_string();
}

std::string FileSystem::GetLastDirectory(const char* path){
	auto parent = GetParentPath(path);
	auto found = parent.find_last_of("/");
	if (found != std::string::npos){
		return parent.substr(found + 1);
	}
	return parent;
}

std::string FileSystem::GetFirstDirectory(const char* path) {
	if (!ValidCString(path)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return{};
	}
	auto strPath = boost::filesystem::path(path).generic_string();
	auto found = strPath.find_first_of("/");
	if (found != std::string::npos) {
		return strPath.substr(0, found);
	}
	return{};
}

std::string FileSystem::ConcatPath(const char* path1, const char* path2){
	if (!ValidCString(path1))
	{
		if (ValidCString(path2))
			return path2;
		else
			return{};
	}
	else {
		if (!ValidCString(path2))
			return path1;
		else {
			auto unifiedpath1 = boost::filesystem::path(path1).generic_string();
			auto unifiedpath2 = boost::filesystem::path(path2).generic_string();
			if (unifiedpath1.back() != '/' && unifiedpath2[0] != '/')
				unifiedpath1.push_back('/');
			return unifiedpath1 + unifiedpath2;
		}
	}
}

std::string FileSystem::UnifyFilepath(const char* path){
	return boost::filesystem::path(path).generic_string();
}

std::string FileSystem::Absolute(const char* path){
	std::string ret = boost::filesystem::absolute(path).generic_string();
	if (ret.size() > 2 && ret.back() == '.')
		ret.erase(ret.end() - 1);
	return ret;
}

std::string FileSystem::AddEndingSlashIfNot(const char* directory){
	if (!ValidCString(directory))
		return std::string();
	std::string ret = boost::filesystem::path(directory).generic_string();
	if (ret.back() != '/')
		ret.push_back('/');
	return ret;
}

std::string FileSystem::RemoveEndingSlash(const char* directory) {
	if (!ValidCString(directory))
		return std::string();
	std::string ret = boost::filesystem::path(directory).generic_string();
	if (ret.back() == '/')
		ret.pop_back();
	return ret;
}

std::string FileSystem::StripFirstDirectoryPath(const char* strFilepath, bool* outStripped){
	using namespace boost::filesystem;
	if(outStripped){
		*outStripped = false;
	}
	std::string ret = path(strFilepath).generic_string();
	auto pos = ret.find_first_of('/');
	if (pos != std::string::npos){
		if (outStripped){
			*outStripped = true;
		}
		ret.erase(ret.begin(), ret.begin() + pos + 1);
	}
	return ret;
}

std::string FileSystem::BackupFile(const char* filepath, unsigned numKeeping) {
	return BackupFile(filepath, numKeeping, "");
}

std::string FileSystem::BackupFile(const char* filepath, unsigned numKeeping, const char* directory){
	boost::filesystem::path path(filepath);
	if (!Exists(filepath))
		return{};
	auto parentPath = path.parent_path().generic_string();
	if (parentPath.empty()) {
		parentPath = "./";
	}
	parentPath = AddEndingSlashIfNot(parentPath.c_str());
	auto filename = path.filename().generic_string();
	std::string backupDir = strlen(directory) != 0  ? AddEndingSlashIfNot(directory) : "";
	auto filenameOnly = FileSystem::ReplaceExtension(filename.c_str(), "");
	auto extension = FileSystem::GetExtension(filepath);
	
	if (!backupDir.empty()) {
		boost::system::error_code ec;
		boost::filesystem::create_directories(boost::filesystem::path(backupDir), ec);
		if (ec) {
			Logger::Log(FB_ERROR_LOG_ARG, ec.message().c_str());
		}
	}

	for (int i = (int)numKeeping - 1; i > 0; --i){
		auto oldPath = FormatString("%s%s_bak%d%s", backupDir.c_str(), filenameOnly.c_str(), i, extension);
		auto newPath = FormatString("%s%s_bak%d%s", backupDir.c_str(), filenameOnly.c_str(), i + 1, extension);		
		FileSystem::Rename(oldPath.c_str(), newPath.c_str());
	}
	auto newPath = FormatString("%s%s_bak%d%s", backupDir.c_str(), filenameOnly.c_str(), 1, extension);
	FileSystem::CopyFile(filepath, newPath.c_str(), true, true);
	return newPath;
}

int FileSystem::CompareFileModifiedTime(const char* file1, const char* file2){
	if (!Exists(file1) || !Exists(file2))
		return FILE_NOT_EXISTS;	
	try{
		auto time1 = boost::filesystem::last_write_time(file1);
		auto time2 = boost::filesystem::last_write_time(file2);
		if (time1 < time2)
			return -1;
		else if (time1 == time2)
			return 0;
		else
			return 1;

	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return 0;
}

int FileSystem::CompareResourceFileModifiedTime(const char* resourceFile, const char* file) {
	auto resourcePath = GetResourcePathIfPathNotExists(resourceFile);
	std::string fba_path;
	auto header = get_fba_header(resourcePath.c_str(), fba_path);
	if (header) {
		auto l = header->modified_time;
		if (!FileSystem::Exists(file)) {
			return FILE_NOT_EXISTS;
		}
		auto r = boost::filesystem::last_write_time(file);
		if (l < r)
			return -1;
		else if (l == r)
			return 0;
		else 
			return 1;
	}	
	return CompareFileModifiedTime(resourcePath.c_str(), file);
}

bool FileSystem::CheckSecurity(const char* filepath){	
	auto abspath = boost::filesystem::absolute(filepath);
	auto testing_folder = abspath.generic_string();
	if (testing_folder.find(g_game_home) != std::string::npos)
		return true;

	auto localFolder = GetAppDataLocalGameFolder();
	if (testing_folder.find(localFolder) != std::string::npos)
		return true;

	auto roamingFolder = GetAppDataRoamingGameFolder();
	if (testing_folder.find(roamingFolder) != std::string::npos)
		return true;

	auto gameFolder = GetMyDocumentGameFolder();
	if (testing_folder.find(gameFolder) != std::string::npos)
		return true;

	auto temp_dir = GetTempDir();
	if (testing_folder.find(temp_dir) != std::string::npos)
		return true;

	return false;
}

ByteArray FileSystem::ReadBinaryFile(const char* path){
	FileSystem::Open file(path, "rb", SharingMode::ReadAllow, ErrorMode::PrintErrorMsg);
	auto err = file.Error();
	if (err) {
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot read(%s)", path).c_str());
		return{};
	}
	fseek(file, 0, SEEK_END);
	auto lSize = ftell(file);
	rewind(file);
	ByteArray buffer(lSize);
	auto result = fread(&buffer[0], 1, lSize, file);
	if (result != lSize) {
		Logger::Log(FB_ERROR_LOG_ARG, "Wrong bytes.");
		return{};
	}
	return buffer;	
}

bool FileSystem::WriteBinaryFile(const char* path, const char* data, size_t length){
	if (!data || length == 0 || path == 0)
		return false;

	if (!CheckSecurity(path))
	{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("FileSystem: SaveBinaryFile to %s has security violation.", path).c_str());
		return false;
	}

	std::ofstream ofs(path, std::ios_base::binary | std::ios_base::trunc);
	if (!ofs)
	{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("FileSystem: Cannot open a file(%s) for writing.", path).c_str());
		return false;
	}
	ofs.write(data, length);
	return true;
}

bool FileSystem::WriteBinaryFile(const char* path, const ByteArray& data)
{
	return WriteBinaryFile(path, (const char*)&data[0], data.size());
}

bool FileSystem::WriteTextFile(const char* path, const char* data, size_t length){
	if (!data || length == 0 || path == 0)
		return false;

	if (!CheckSecurity(path))
	{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("FileSystem: SaveBinaryFile to %s has security violation.", path).c_str());
		return false;
	}

	std::ofstream ofs(path, std::ios_base::trunc);
	if (!ofs)
	{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("FileSystem: Cannot open a file(%s) for writing.", path).c_str());
		return false;
	}
	ofs.write(data, length);
	return true;
}

namespace fb {
	static SpinLock<true, true> sDeleteOnExitGuard;
	static std::vector<std::string> sDeleteOnExit;
	class DeleteOnExit {
	public:
		DeleteOnExit() {
		}
		~DeleteOnExit() {
			for (auto& path : sDeleteOnExit) {
				if (FileSystem::CheckSecurity(path.c_str())) {
					FileSystem::Remove(path.c_str());
				}
			}
			sDeleteOnExit.clear();
		}
		
	};
}

static DeleteOnExit DeleteOnExitInst;

void FileSystem::DeleteOnExit(const char* path) {
	EnterSpinLock<SpinLock<true, true>> lock(sDeleteOnExitGuard);
	sDeleteOnExit.push_back(path);
}

void FileSystem::_PrepareQuit(){
	sDeleteOnExit.clear();
}

bool FileSystem::IsFileOutOfDate(const char* path, time_t expiryTime) {
	if (!ValidCString(path)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return false;
	}
	if (IsOpaqueURI(path))
		return false;

	return Exists(path) && GetLastModified(path) < expiryTime;		
}

bool FileSystem::IsOpaqueURI(const char* uri) {
	if (!ValidCString(uri)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return false;
	}

	auto len = strlen(uri);
	for (size_t i = 0; i < len; ++i) {
		if (uri[i] == '/')
			return false;
	}
	return true;
}

bool FileSystem::IsFileOutOfDate(const char* url, size_t expiryTime) {
	if (!ValidCString(url))
	{
		Logger::Log(FB_ERROR_LOG_ARG, "URLIsNull");
		return false;
	}

	try
	{
		// Determine whether the file can be treated like a File, e.g., a jar entry.		
		if (IsOpaqueURI(url))
			return false; // TODO: Determine how to check the date of non-Files

		auto file = File::From(url);

		return file->Exists() && file->LastModified() < expiryTime;
	}
	catch (std::exception& e)
	{
		Logger::Log(FB_ERROR_LOG_ARG, "Exception occurred : %s", e.what());
		return false;
	}
}

std::shared_ptr<tinyxml2::XMLDocument> FileSystem::LoadXml(const char* path) {
	Open file(path, "r", ReadAllow, PrintErrorMsg);
	auto doc = std::make_shared<tinyxml2::XMLDocument>();
	auto& text = file.GetTextData();	
	doc->Parse(text.c_str());	
	return doc;
}

//---------------------------------------------------------------------------
// Directory Operataions
//---------------------------------------------------------------------------
DirectoryIteratorPtr FileSystem::GetDirectoryIterator(const char* filepath, bool recursive){
	if (!Exists(filepath))
	{
		// check fba
		pack_datum* pack = get_pack_datum_containing(filepath);
		if (pack) {
			return DirectoryIteratorPtr(new DirectoryIterator(filepath, recursive, pack),
				[](DirectoryIterator* obj) {delete obj; });
		}

		return nullptr;
	}
	return DirectoryIteratorPtr(new DirectoryIterator(filepath, recursive),
		[](DirectoryIterator* obj){delete obj; });
}

bool FileSystem::CreateDirectory(const char* filepath){
	std::string path = boost::filesystem::path(filepath).generic_string();
	if (StartsWith(path, "./")) {
		path = path.substr(2);
	}
	bool ret = false;
	try{
		if (FindLastIndexOf(path.c_str(), '.') != -1) {
			auto parentPath = GetParentPath(path.c_str());
			ret = boost::filesystem::create_directories(parentPath.c_str());
		}
		else {
			ret = boost::filesystem::create_directories(path.c_str());
		}
	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return ret;
}


//---------------------------------------------------------------------------
// System Folders
//---------------------------------------------------------------------------
void FileSystem::SetApplicationName(const char* name){
	gApplicationName = name;
}

std::string AppendGameFolder(const std::string& known_folder) {	
	std::string game_folder = FileSystem::AddEndingSlashIfNot(known_folder.c_str());	
	if (!gApplicationName.empty()) {
		game_folder += gApplicationName;
		game_folder += "/";
	}
	else {
		game_folder += "fb_game/";
	}
	return game_folder;
}
std::string FileSystem::GetMyDocumentGameFolder(){
#if defined(_PLATFORM_WINDOWS_)
	PWSTR path=0;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, 0, &path))){
		auto ret = std::string(WideToAnsi(path));
		ret += "\\my games\\";
		auto game_folder = AppendGameFolder(ret);		
		CoTaskMemFree(path);
		boost::filesystem::path p(game_folder);
		return p.generic_string();		
	}	
#else
	assert(0 && "Not implemented");
#endif
	Logger::Log(FB_ERROR_LOG_ARG, "Cannot retrieve MyDocumentGameFolder.");
	return std::string("/temp/documents/fb_game/");
}

std::string FileSystem::GetAppDataRoamingGameFolder() {
#if defined(_PLATFORM_WINDOWS_)
	PWSTR path = 0;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, 0, &path))) {
		auto ret = std::string(WideToAnsi(path));
		auto game_folder = AppendGameFolder(ret);
		CoTaskMemFree(path);
		boost::filesystem::path p(game_folder);
		return p.generic_string();
	}

#else
	assert(0 && "Not implemented");
#endif
	return std::string("./temp/appdata_roaming/fb_game/");
}

std::string FileSystem::GetAppDataLocalGameFolder() {
#if defined(_PLATFORM_WINDOWS_)
	PWSTR path = 0;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, 0, &path))) {
		auto ret = std::string(WideToAnsi(path));
		auto game_folder = AppendGameFolder(ret);
		CoTaskMemFree(path);
		boost::filesystem::path p(game_folder);
		return p.generic_string();
	}

#else
	assert(0 && "Not implemented");
#endif
	return std::string("./temp/appdata_local/fb_game/");
}


std::string FileSystem::GetCurrentDir(){
	if (gWorkingPath.empty()){
		gWorkingPath = boost::filesystem::current_path(); // absolute
	}
	return gWorkingPath.generic_string();
}

std::string FileSystem::GetTempDir() {
	auto tmp = boost::filesystem::temp_directory_path().generic_string();
	tmp = AppendGameFolder(tmp.c_str());	
	return tmp;
}

void FileSystem::SetCurrentDir(const char* path) {
	try {
		boost::filesystem::current_path(path);
	}
	catch (const boost::filesystem::filesystem_error& error) {
		Logger::Log(FB_ERROR_LOG_ARG, error.what());
	}
}

std::string FileSystem::TempFileName(const char* prefix, const char* suffix) {
	auto cwd = ConcatPath(GetCurrentDir().c_str(), "/tmp/");
	char buffer[L_tmpnam];
	
	if (ValidCString(prefix))
		cwd += prefix;

	tmpnam_s(buffer);
	cwd += buffer;

	if (ValidCString(suffix))
		cwd += suffix;
	return cwd;
}

static std::string ILLEGAL_FILE_PATH_PART_CHARACTERS = "[?=+<>:;,\"^";
void FileSystem::ReplaceIllegalFileNameCharacters(std::string& filepath) {
	std::replace_if(filepath.begin(), filepath.end(), [](char c) {
		for (auto ic : ILLEGAL_FILE_PATH_PART_CHARACTERS) {
			if (ic == c)
				return true;
		}
		return false;},
		'_');
}

bool FileSystem::CanWrite(const char* filepath) {
	FileSystem::Open file(filepath, "a+", ReadAllow, SkipErrorMsg);
	auto err = file.Error();
	if (err) {
		return false;
	}
	return true;
}

void FileSystem::SetLastModified(const char* filepath) {
	try {
		boost::filesystem::last_write_time(filepath, time(NULL));
	}
	catch (boost::filesystem::filesystem_error& error){
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Cannot change last write time : %s", error.what()));
	}
}

time_t FileSystem::GetLastModified(const char* filepath) {
	try {
		return boost::filesystem::last_write_time(filepath);
	}
	catch (boost::filesystem::filesystem_error& error) {
		Logger::Log(FB_ERROR_LOG_ARG, 
			"Cannot get last write time for : %s(%s)", filepath, error.what());
	}
	return 0;
}

std::string FileSystem::FormPath(int n, ...) {
	std::string ret;
	va_list vl;
	va_start(vl, n);
	std::string val;
	for (int i = 0; i<n; i++)
	{
		val = va_arg(vl, std::string);
		if (!val.empty()) {
			ReplaceIllegalFileNameCharacters(val);
			ret += val;
			ret += "/";
		}		
	}
	va_end(vl);
	return ret;
}

void FileSystem::AddResourceFolder(const char* startingPath, const char* res) {
	if (!ValidCString(startingPath) || !ValidCString(res))
	{
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return;
	}

	if (!Exists(res)) {
		auto path = FileSystem::UnifyFilepath(res);		
		// result: D:/Projects/fastbird-engine/EssentialEngineData
		auto fba_path = FileSystem::RemoveEndingSlash(path.c_str());
		fba_path += ".fba";
		if (!Exists(fba_path.c_str())) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Resource path(%s) does not exists.", res).c_str());
			return;
		}
		else {
			auto ppath = FileSystem::GetParentPath(fba_path.c_str());
			if (parse_fba(ppath.c_str()) == 0) {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Failed to load resource pack file(%s).", fba_path.c_str()).c_str());
				return;
			}
		}		
	}
	sResourceFolders[startingPath] = res;		
	sResourceFoldersLower[ToLowerCase(startingPath)] = res;
}

void FileSystem::RemoveResourceFolder(const char* startingPath) {
	if (!ValidCString(startingPath)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg");
		return;
	}
	sResourceFolders.erase(startingPath);
	sResourceFoldersLower.erase(ToLowerCase(startingPath));
}

const char* FileSystem::GetResourceFolder(const char* startingPath) {
	if (!ValidCString(startingPath)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return "";
	}
	
	auto it = sResourceFolders.find(startingPath);
	if (it != sResourceFolders.end()) {
		if (strcmp(GetExtensionWithOutDot(it->second.c_str()), FBRExt) == 0) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not a folder. It is a fbr.", startingPath).c_str());
			return "";
		}
		else {
			return it->second.c_str();
		}
	}
	{
		auto loweredPath = ToLowerCase(startingPath);
		auto it = sResourceFoldersLower.find(loweredPath);
		if (it != sResourceFoldersLower.end()) {
			if (strcmp(GetExtensionWithOutDot(it->second.c_str()), FBRExt) == 0) {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not a folder. It is a fbr.", startingPath).c_str());
				return "";
			}
			else {
				return it->second.c_str();
			}
		}
	}

	return "";
}

bool FileSystem::IsResourceFolderByKey(const char* startingPath) {
	auto strPath = boost::filesystem::path(startingPath).generic_string();
	if (strPath.back() != '/') {
		strPath.push_back('/');
	}

	return ValidCString(GetResourceFolder(strPath.c_str()));
}

bool FileSystem::IsResourceFolderByVal(const char* resourcePath) {
	for (auto& it : sResourceFolders) {
		if (strcmp(it.second.c_str(), resourcePath)==0) {
			return true;
		}
	}
	auto lowered = ToLowerCase(resourcePath);
	for (auto& it : sResourceFoldersLower) {
		if (strcmp(it.second.c_str(), lowered.c_str()) == 0) {
			return true;
		}
	}
	return false;
}

std::string FileSystem::GetResourcePath(const char* path) {
	for (auto& it : sResourceFolders) {
		if (StartsWith(path, it.first.c_str(), true)) {
			std::string newPath = it.second;
			newPath += boost::filesystem::path(path).generic_string().substr(it.first.size());
			return newPath;
		}
	}
	return{};
}

std::string FileSystem::GetResourcePathIfPathNotExists(const char* path) {
	if (Exists(path))
		return path;
	auto resourcePath = GetResourcePath(path);
	if (resourcePath.empty())
		return path;
	else
		return resourcePath;
}

std::string FileSystem::GetResourceKeyFromVal(const char* val) {
	for (auto& it : sResourceFolders) {
		if (strcmp(it.second.c_str(), val) == 0) {
			return it.first;
		}
	}
	return{};
}

errno_t FileSystem::OpenResourceFile(FILE** f, const char* path, const char* mode, ByteArrayPtr& fbdata) {
	fbdata = 0;
	errno_t err;
	
	err = fopen_s(f, path, mode);
	if (err) {
		fbdata = FileSystem::get_fba_file_data(path);
		if (fbdata) {
			err = 0;
		}
	}
	if (err) {
		auto resourcePath = GetResourcePath(path);
		if (!resourcePath.empty()) {
			err = fopen_s(f, resourcePath.c_str(), mode);
			if (err) {
				fbdata = FileSystem::get_fba_file_data(resourcePath.c_str());
				if (fbdata) {
					err = 0;
				}
			}
		}
	}
	return err;
}

FileSystem::Open::Open()
	: mFile(0)
	, mErr(0)
	, mCurrentPosition(0)
	, mMemoryFile(false)
{

}

FileSystem::Open::Open(const char* path, const char* mode)
	: Open()
{
	operator()(path, mode, PrintErrorMsg);
}

FileSystem::Open::Open(const char* path, const char* mode, ErrorMode errorMsgMode)
	: Open()
{
	operator()(path, mode, errorMsgMode);
}

FileSystem::Open::Open(const char* path, const char* mode, SharingMode share, ErrorMode errorMsgMode)
	:Open()
{
	operator()(path, mode, share, errorMsgMode);
}


FileSystem::Open::Open(Open&& other) _NOEXCEPT
	: mFile(other.Release())
	, mErr(other.mErr)
{
}

FileSystem::Open::~Open()
{
	Close();
}

errno_t FileSystem::Open::Reset(const char* path, const char* mode) {
	Close();
	return operator()(path, mode, PrintErrorMsg);	
}

errno_t FileSystem::Open::Reset(const char* path, const char* mode, ErrorMode errorMsgMode) {
	Close();
	return operator()(path, mode, errorMsgMode);
}
errno_t FileSystem::Open::Reset(const char* path, const char* mode, SharingMode share, ErrorMode errorMsgMode) {
	Close();
	return operator()(path, mode, share, errorMsgMode);
}

bool FileSystem::Open::IsOpen() {
	return mFile != 0 || mMemoryFile;
}

void FileSystem::Open::Close()
{
	if (mFile) {
		fclose(mFile);
		mFile = 0;		
	}
	mErr = 0;
	ClearWithSwap(mBinaryData);
	ClearWithSwap(mTextData);
	mCurrentPosition = 0;
}

FILE* FileSystem::Open::Release()
{
	auto ret = mFile;
	mFile = 0;	
	return ret;
}

errno_t FileSystem::Open::operator()(const char* path, const char* mode)
{
	return operator()(path, mode, PrintErrorMsg);
}

errno_t FileSystem::Open::operator()(const char* path, const char* mode, ErrorMode errorMsgMode)
{
	return operator()(path, mode, NoSharing, errorMsgMode);
}

errno_t FileSystem::Open::operator()(const char* path, const char* mode, SharingMode share, ErrorMode errorMsgMode)
{
	Close();
	mFilePath = path;
	
	mErr = TryOpenFile(path, mode, share);

	using namespace std::chrono_literals;
	if (mErr && !mMemoryFile) {
#if defined(_PLATFORM_WINDOWS_)
		if (mErr == 13 && share== NoSharing) {// permission denied
			int numTry = 0;
			while (numTry++ < 10 && mErr == 13) {
				std::this_thread::sleep_for(20ms);
				mErr = TryOpenFile(path, mode, share);
				if (IsOpen())
					break;
			}
		}
#else
#endif
	}
	
	if (mErr && !mMemoryFile && errorMsgMode == PrintErrorMsg) {
		char errString[512] = {};
		strerror_s(errString, mErr);
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open a file(%s): error(%d): %s",
			path, mErr, errString).c_str());
	}

	return mErr;
}

errno_t FileSystem::Open::TryOpenFile(const char* path, const char* mode, SharingMode share) {
	if (share != NoSharing) {
		int smode = 0;
		if (!(share & ReadAllow)) {
			smode = _SH_DENYRD;
		}
		if (!(share & WriteAllow)) {
			smode += _SH_DENYWR;
		}
		if (!smode) {
			smode = _SH_DENYNO;
		}

		mFile = _fsopen(path, mode, smode);
		if (!mFile) {
			mBinaryData = FileSystem::get_fba_file_data(path);
			if (mBinaryData) {
				mMemoryFile = true;
				mErr = 0;
			}
		}
		if (!mMemoryFile && !mFile) {
			auto resourcePath = GetResourcePath(path);
			if (!resourcePath.empty()) {
				mFile = _fsopen(resourcePath.c_str(), mode, smode);
				if (!mFile) {
					mBinaryData = FileSystem::get_fba_file_data(resourcePath.c_str());
					if (mBinaryData) {
						mMemoryFile = true;
						mErr = 0;
					}
				}
			}
			if (!mFile && !mMemoryFile) {
				mErr = errno;
			}
		}
	}
	else {
		mErr = OpenResourceFile(&mFile, path, mode, mBinaryData);
		if (mBinaryData) {
			mErr = 0;
			mMemoryFile = true;
		}
	}
	return mErr;
}

//FileSystem::Open::operator FILE* () const
//{
//	if (!mFile && mFileData) {
//		Logger::Log(FB_ERROR_LOG_ARG, "This file is in the memory. Use GetFileData()");
//	}
//	return mFile;
//}

ByteArrayPtr FileSystem::Open::GetBinaryData() {
	if (!mBinaryData && mFile) {
		fseek(mFile, 0, SEEK_END);
		auto size = ftell(mFile);
		fseek(mFile, 0, SEEK_SET);
		mBinaryData = std::make_shared<ByteArray>(size);		
		if (size == 0) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Zero length file(%s) found.", size).c_str());
			return mBinaryData;
		}
		auto& data = *mBinaryData;
		auto got = fread(&data[0], 1, size, mFile);
		if (got != size) {
			Logger::Log(FB_ERROR_LOG_ARG, "Ascii file treated as binary.");
		}
		if (ferror(mFile)) {
			Logger::Log(FB_ERROR_LOG_ARG, "Error reading file.");
		}
	}

	return mBinaryData;
}
const std::string& FileSystem::Open::GetTextData() {
	if (mTextData.empty() && mBinaryData) {
		mTextData.insert(mTextData.begin(), mBinaryData->begin(), mBinaryData->end());
#if defined(_PLATFORM_WINDOWS_)
		ReplaceAll(mTextData, "\r\n", "\n");
#endif
	}
	else if (mTextData.empty() && mFile) {
		char buffer[65536];
		size_t readsize = 0;
		while (readsize = fread(buffer, 1, 65535, mFile)) {
			buffer[readsize] = 0;
			mTextData += buffer;
		}
		if (ferror(mFile)) {
			Logger::Log(FB_ERROR_LOG_ARG, "Error reading file.");
		}
	}

	return mTextData;
}

bool FileSystem::Open::IsMemoryFile() {
	return mMemoryFile;
}

errno_t FileSystem::Open::Error() const
{
	return mErr;
}

int FileSystem::Open::eof() {
	if (mMemoryFile) {
		return (mCurrentPosition == mBinaryData->size()) ? 1 : 0;
	}
	else {
		return feof(mFile);
	}
}

FILE* FileSystem::OpenFile(const char* path, const char* mode, errno_t* errorNo)
{
	if (errorNo)
		*errorNo = 0;
	FILE* f = 0;
	auto err = fopen_s(&f, path, mode);
	if (err) {
		if (errorNo) {
			*errorNo = err;
		}
		char errString[512] = {};
		strerror_s(errString, err);
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open a file(%s): error(%d): %s",
			path, err, errString).c_str());
	}
	return f;
}

FILE* FileSystem::OpenFileShared(const char* path, const char* mode, SharingMode sharingMode, errno_t* errorNo)
{
	if (errorNo)
		*errorNo = 0;
	int smode = 0;
	if (!(sharingMode & ReadAllow)) {
		smode = _SH_DENYRD;
	}
	if (!(sharingMode & WriteAllow)) {
		smode += _SH_DENYWR;
	}
	if (!smode) {
		smode = _SH_DENYNO;
	}
	FILE* f = _fsopen(path, mode, smode);
	if (!f) {
		auto errNo = errno;
		if (errorNo) {
			*errorNo = errNo;
		}
		char errString[512] = {};
		strerror_s(errString, errNo);
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open a file(%s): error(%d): %s",
			path, errNo, errString).c_str());
	}
	return f;
}

void FileSystem::CloseFile(FILE* &file)
{
	if (!file)
		return;
	fclose(file);
	file = 0;
}

FILE* FileSystem::OpenFileByMode(const char* path, const char* mode, errno_t* errorNo)
{
	if (strchr(mode, 'r')) {
		return FileSystem::OpenFileShared(path, mode, FileSystem::ReadAllow, errorNo);
	}
	else {
		return FileSystem::OpenFile(path, mode, errorNo);
	}
}

FileSystem::Lock::Lock() {
	sGuard.Lock();
}
FileSystem::Lock::~Lock() {
	sGuard.Unlock();
}
namespace fb {
	FB_DLL_FILESYSTEM
	size_t fread(void* buffer, size_t element_size, size_t element_count, FileSystem::Open& file) {
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, "File is not open.");
			return 0;
		}
		if (file.mMemoryFile) {
			auto& data = *file.GetBinaryData();
			auto readlen = element_size * element_count;
			auto end = file.mCurrentPosition + readlen;
			if (end >= data.size()) {
				readlen = data.size() - file.mCurrentPosition;
			}
			memcpy(buffer, &data[0] + file.mCurrentPosition, readlen);
			file.mCurrentPosition += readlen;
			return readlen;
		}
		else {
			return ::fread(buffer, element_size, element_count, file.mFile);
		}
	}

	FB_DLL_FILESYSTEM
	size_t fread_s(void* buffer, size_t buffer_size, size_t element_size, size_t count, FileSystem::Open& file) {
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, "File is not open.");
			return 0;
		}
		if (file.mMemoryFile) {
			auto& data = *file.GetBinaryData();
			auto readlen = element_size * count;
			auto end = file.mCurrentPosition + readlen;
			if (end >= data.size()) {
				readlen = data.size() - file.mCurrentPosition;
			}
			if (readlen > buffer_size) {
				readlen = buffer_size;
			}
			memcpy(buffer, &data[0] + file.mCurrentPosition, readlen);
			file.mCurrentPosition += readlen;
			return readlen;
		}
		else {
			return ::fread_s(buffer, buffer_size, element_size, count, file.mFile);
		}
	}

	FB_DLL_FILESYSTEM
	size_t fseek(FileSystem::Open& file, long offset, int origin) {
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, "File is not open");
			return -1;
		}
		if (file.mMemoryFile) {
			if (origin == SEEK_SET) {
				if (offset > (long)file.mBinaryData->size() || offset < 0) {
					Logger::Log(FB_ERROR_LOG_ARG, "Invalid offset.");
					return 1;
				}
				file.mCurrentPosition = offset;
				return 0;
			}
			else if (origin == SEEK_CUR) {
				auto result_pos = file.mCurrentPosition + offset;
				if (result_pos > (long)file.mBinaryData->size() || result_pos < 0) {
					Logger::Log(FB_ERROR_LOG_ARG, "Invalid offset.");
					return 1;
				}
				file.mCurrentPosition = result_pos;
				return 0;
			}
			else if (origin == SEEK_END) {
				auto result_pos = file.mBinaryData->size() + offset;
				if (result_pos > file.mBinaryData->size() || result_pos < 0) {
					Logger::Log(FB_ERROR_LOG_ARG, "Invalid offset.");
					return 1;
				}
				file.mCurrentPosition = result_pos;
				return 0;
			}
			else {
				return 2;
			}
		}
		else {
			return ::fseek(file.mFile, offset, origin);
		}
	}

	FB_DLL_FILESYSTEM
	int feof(FileSystem::Open& file) {
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, "File not open.");
			return 0;
		}

		if (file.mMemoryFile) {
			return (file.mCurrentPosition >= (long long)file.mBinaryData->size()) ? 1 : 0;
		}
		else {
			return ::feof(file.mFile);
		}
	}

	FB_DLL_FILESYSTEM
	long int ftell(FileSystem::Open& file) {
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, "File not open.");
			return -1L;
		}

		if (file.mMemoryFile) {
			return file.mCurrentPosition;
		}
		else {
			return ftell(file.mFile);
		}
	}

	FB_DLL_FILESYSTEM
	void rewind(FileSystem::Open& file) {
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, "File not open.");
			return;
		}
		if (file.mMemoryFile) {
			file.mCurrentPosition = 0;
		}
		else {
			rewind(file.mFile);
		}
	}
}