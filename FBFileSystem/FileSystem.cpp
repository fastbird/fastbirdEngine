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
using namespace fb;

static bool gLogginStarted = false;
static bool gSecurityCheck = true;
static boost::filesystem::path gWorkingPath;
std::string gApplicationName;
static std::unordered_map<std::string, std::string> sResourceFolders;
#define FBRExt "fbr"
static RecursiveSpinLock<true, true> sGuard;
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

void FileSystem::SetSecurityCheck(bool enable) {
	gSecurityCheck = enable;
}

bool FileSystem::Exists(const char* path){
	//FileSystem::Lock l;
	boost::system::error_code err;
	return boost::filesystem::exists(path, err);
}

bool FileSystem::ResourceExists(const char* path, std::string* outResoucePath) {
	//FileSystem::Lock l;
	boost::system::error_code err;
	bool ex = boost::filesystem::exists(path, err);
	if (ex) {
		if (outResoucePath)
			*outResoucePath = path;
		return ex;
	}
	else{
		auto resourcePath = GetResourcePath(path);
		boost::system::error_code err;
		ex = boost::filesystem::exists(resourcePath, err);
		if (ex) {
			if (outResoucePath)
				*outResoucePath = resourcePath;
			return ex;
		}
	}
	return ex;
}

bool FileSystem::IsDirectory(const char* path){
	return boost::filesystem::is_directory(path);
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

	if (Exists(newpath)){
		return RENAME_DEST_EXISTS;
	}
	
	if (!SecurityOK(path) || !SecurityOK(newpath))
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
	Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) removing all files in %s", path).c_str());
	size_t ret = true;
	try {
		ret = (size_t)boost::filesystem::remove_all(path);
	}
	catch (boost::filesystem::filesystem_error& err) {
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return ret;
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

std::string FileSystem::MakrEndingSlashIfNot(const char* directory){
	if (!ValidCString(directory))
		return std::string();
	std::string ret = boost::filesystem::path(directory).generic_string();
	if (ret.back() != '/')
		ret.push_back('/');
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

void FileSystem::BackupFile(const char* filepath, unsigned numKeeping) {
	BackupFile(filepath, numKeeping, "./");
}

void FileSystem::BackupFile(const char* filepath, unsigned numKeeping, const char* directory){
	boost::filesystem::path path(filepath);
	auto parentPath = path.parent_path().generic_string();
	if (parentPath.empty()) {
		parentPath = "./";
	}
	parentPath = MakrEndingSlashIfNot(parentPath.c_str());
	auto filename = path.filename().generic_string();
	std::string backupDir = MakrEndingSlashIfNot(directory);
	auto filenameOnly = FileSystem::ReplaceExtension(filename.c_str(), "");
	auto extension = FileSystem::GetExtension(filepath);
	for (int i = (int)numKeeping - 1; i > 0; --i){
		auto oldPath = FormatString("%s%s%s_bak%d%s", parentPath.c_str(), backupDir.c_str(), filenameOnly.c_str(), i, extension);
		auto newPath = FormatString("%s%s%s_bak%d%s", parentPath.c_str(), backupDir.c_str(), filenameOnly.c_str(), i + 1, extension);
		boost::filesystem::create_directories(boost::filesystem::path(newPath).parent_path());
		FileSystem::Rename(oldPath.c_str(), newPath.c_str());
	}
	auto newPath = FormatString("%s%s%s_bak%d%s", parentPath.c_str(), backupDir.c_str(), filenameOnly.c_str(), 1, extension);
	FileSystem::CopyFile(filepath, newPath.c_str(), true, true);
}

int FileSystem::CompareFileModifiedTime(const char* file1, const char* file2){
	if (!Exists(file1) || !Exists(file2))
		return FILE_NO_EXISTS;	
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
	return CompareFileModifiedTime(resourcePath.c_str(), file);
}

bool FileSystem::SecurityOK(const char* filepath){
	if (!gSecurityCheck)
		return true;

	auto cwd = GetCurrentDir();
	auto abspath = boost::filesystem::absolute(filepath);
	if (abspath.generic_string().find(cwd) != std::string::npos)
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

	if (!SecurityOK(path))
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

	if (!SecurityOK(path))
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
				if (FileSystem::SecurityOK(path.c_str())) {
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


//---------------------------------------------------------------------------
// Directory Operataions
//---------------------------------------------------------------------------
DirectoryIteratorPtr FileSystem::GetDirectoryIterator(const char* filepath, bool recursive){
	return DirectoryIteratorPtr(new DirectoryIterator(filepath, recursive),
		[](DirectoryIterator* obj){delete obj; });
}

bool FileSystem::CreateDirectory(const char* filepath){
	bool ret = false;
	try{
		if (FindLastIndexOf(filepath, '.') != -1) {
			auto parentPath = GetParentPath(filepath);
			ret = boost::filesystem::create_directories(parentPath.c_str());
		}
		else {
			ret = boost::filesystem::create_directories(filepath);
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

std::string FileSystem::GetAppDataFolder(){
#if defined(_PLATFORM_WINDOWS_)
	PWSTR path=0;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, 0, &path))){
		auto ret = std::string(WideToAnsi(path));
		if (!gApplicationName.empty())
			ret += "\\my games\\" + gApplicationName;
		CoTaskMemFree(path);
		boost::filesystem::path p(ret);
		return p.generic_string();		
	}	
#else
	assert(0 && "Not implemented");
#endif
	return std::string("./temp/");
}


std::string FileSystem::GetCurrentDir(){
	if (gWorkingPath.empty()){
		gWorkingPath = boost::filesystem::current_path(); // absolute
	}
	return gWorkingPath.generic_string();
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
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Resource(%s) does not exist.", res).c_str());
		return;
	}
	sResourceFolders[startingPath] = res;
}

void FileSystem::RemoveResourceFolder(const char* startingPath) {
	if (!ValidCString(startingPath)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg");
		return;
	}
	sResourceFolders.erase(startingPath);
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
	return false;
}

std::string FileSystem::GetResourcePath(const char* path) {
	for (auto& it : sResourceFolders) {
		if (StartsWith(path, it.first.c_str())) {
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

errno_t FileSystem::OpenResourceFile(FILE** f, const char* path, const char* mode) {
	auto err = fopen_s(f, path, mode);
	if (err) {
		auto resourcePath = GetResourcePath(path);
		if (!resourcePath.empty()) {
			err = fopen_s(f, resourcePath.c_str(), mode);			
		}
	}
	return err;
}

FileSystem::Open::Open()
	: mFile(0)
	, mErr(0)
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
	return mFile != 0;
}

void FileSystem::Open::Close()
{
	if (mFile) {
		fclose(mFile);
		mFile = 0;		
	}
	mErr = 0;
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
			auto resourcePath = GetResourcePath(path);
			if (!resourcePath.empty()) {
				mFile = _fsopen(resourcePath.c_str(), mode, smode);
			}
			if (!mFile) {
				mErr = errno;
			}
		}
	}
	else {
		mErr = OpenResourceFile(&mFile, path, mode);
	}	

	if (mErr && errorMsgMode == PrintErrorMsg) {
		char errString[512] = {};
		strerror_s(errString, mErr);
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open a file(%s): error(%d): %s",
			path, mErr, errString).c_str());
	}

	return mErr;
}

FileSystem::Open::operator FILE* () const
{
	return mFile;
}

errno_t FileSystem::Open::Error() const
{
	return mErr;
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