#include "stdafx.h"
#include "Network.h"
#include "Utils.h"
#include "boost/network/uri/uri.hpp"
namespace fb {
	std::map<std::string, std::string> mimeTypeToSuffixMap = {
{"application/acad", "dwg"},
{"application/bil", "bil"},
{"application/bil16", "bil"},
{"application/bil32", "bil"},
{"application/dxf", "dxf"},
{"application/octet-stream", "bin"},
{"application/pdf", "pdf"},
{"application/rss+xml", "xml"},
{"application/rtf", "rtf"},
{"application/sla", "slt"},
{"application/vnd.google-earth.kml+xml", "kml"},
{"application/vnd.google-earth.kmz", "kmz"},
{"application/vnd.ogc.gml+xml", "gml"},
{"application/x-gzip", "gz"},
{"application/xml", "xml"},
{"application/zip", "zip"},
{"multipart/zip", "zip"},
{"multipart/x-gzip", "gzip"},

{"model/collada+xml", "dae"},   // <--- burkey add

{"text/html", "html"},
{"text/plain", "txt"},
{"text/richtext", "rtx"},
{"text/tab-separated-values", "tsv"},
{"text/xml", "xml"},

{"image/bmp", "bmp"},
{"image/dds", "dds"},
{"image/geotiff", "gtif"},
{"image/gif", "gif"},
{"image/jp2", "jp2"},
{"image/jpeg", "jpg"},
{"image/jpg", "jpg"},
{"image/png", "png"},
{"image/svg+xml", "svg"},
{"image/tiff", "tif"},
{"image/x-imagewebserver-ecw", "ecw"},
{"image/x-mrsid", "sid"},
{"image/x-rgb", "rgb"},

{"video/mpeg", "mpg"},
{"video/quicktime", "mov"},

{"audio/x-aiff", "aif"},
{"audio/x-midi", "mid"},
{"audio/x-wav", "wav"},

{"world/x-vrml", "wrl"},
	};

	std::map<std::string, std::string> suffixToMimeTypeMap = {
	{"aif", "audio/x-aiff"},
	{"aifc", "audio/x-aiff"},
	{"aiff", "audio/x-aiff"},
	{"bil", "application/bil"},
	{"bil16", "application/bil16"},
	{"bil32", "application/bil32"},
	{"bin", "application/octet-stream"},
	{"bmp", "image/bmp"},
	{"dds", "image/dds"},
	{"dwg", "application/acad"},
	{"dxf", "application/dxf"},
	{"ecw", "image/x-imagewebserver-ecw"},
	{"gif", "image/gif"},
	{"gml", "application/vnd.ogc.gml+xml"},
	{"gtif", "image/geotiff"},
	{"gz", "application/x-gzip"},
	{"gzip", "multipart/x-gzip"},
	{"htm", "text/html"},
	{"html", "text/html"},
	{"jp2", "image/jp2"},
	{"jpeg", "image/jpeg"},
	{"jpg", "image/jpeg"},
	{"kml", "application/vnd.google-earth.kml+xml"},
	{"kmz", "application/vnd.google-earth.kmz"},
	{"mid", "audio/x-midi"},
	{"midi", "audio/x-midi"},
	{"mov", "video/quicktime"},
	{"mp3", "audio/x-mpeg"},
	{"mpe", "video/mpeg"},
	{"mpeg", "video/mpeg"},
	{"mpg", "video/mpeg"},
	{"pdf", "application/pdf"},
	{"png", "image/png"},
	{"rgb", "image/x-rgb"},
	{"rtf", "application/rtf"},
	{"rtx", "text/richtext"},
	{"sid", "image/x-mrsid"},
	{"slt", "application/sla"},
	{"svg", "image/svg+xml"},
	{"tif", "image/tiff"},
	{"tiff", "image/tiff"},
	{"tsv", "text/tab-separated-values"},
	{"txt", "text/plain"},
	{"wav", "audio/x-wav"},
	{"wbmp", "image/vnd.wap.wbmp"},
	{"wrl", "world/x-vrml"},
	{"xml", "application/xml"},
	{"zip", "application/zip"},
	};

	int Network::ErrorCode = 0;

	std::string Network::MakeMimeTypeForSuffix(const char* suffix) {
		if (!ValidCStringLength(suffix))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "invalid arg.");
			return std::string();
		}

		std::string strSuffix;
		// Strip the starting period from the suffix string, if any exists.
		if (suffix[0] == '.') {
			auto dotExcluded = suffix + 1;
			strSuffix = ToLowerCase(dotExcluded);

		}
		else {
			strSuffix = ToLowerCase(suffix);
		}
		auto it = suffixToMimeTypeMap.find(strSuffix);
		if (it != suffixToMimeTypeMap.end()) {
			return it->second;
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Cannot find mime type for suffix(%s)", suffix).c_str());
			return std::string();
		}
	}

	std::string Network::MakeSuffixForMimeType(const char* mimeType) {
		if (!ValidCStringLength(mimeType))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return std::string();
		}
		auto len = strlen(mimeType);
		if (!strchr(mimeType, '/') || mimeType[len - 1] == '/')
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid format");
			return std::string();
		}

		// Remove any parameters appended to this mime type before using it as a key in the mimeTypeToSuffixMap. Mime
		// parameters do not change the mapping from mime type to suffix.
		int paramIndex = -1;
		auto semiPos = strchr(mimeType, ';');
		if (semiPos) {
			paramIndex = semiPos - mimeType;
		}
		std::string strMimeType;
		if (paramIndex != -1) {
			strMimeType = SubString(mimeType, 0, paramIndex);
		}
		else {
			strMimeType = mimeType;
		}

		std::string suffix;
		auto it = mimeTypeToSuffixMap.find(strMimeType);
		if (it != mimeTypeToSuffixMap.end())
		{
			suffix = it->second;
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Cannot find the suffix for the mimeType %s", strMimeType.c_str()).c_str());
		}


		if (suffix.empty())
			suffix = strMimeType.substr(strMimeType.find_last_of('/') + 1);

		ReplaceAll(suffix, "bil32", "bil");
		ReplaceAll(suffix, "bil16", "bil");

		return "." + suffix;
	}

	ByteArray Network::readURLContentToBuffer(const std::string& url) {
		std::string contentType;
		return readURLContentToBuffer(url, contentType, false);
	}

	ByteArray Network::readURLContentToBuffer(const std::string& url, std::string& contentType)
	{
		return readURLContentToBuffer(url, contentType, false);
	}

	ByteArray Network::readURLContentToBuffer(const std::string& url, std::string& contentType, bool allocateDirect) {
		ErrorCode = NoError;
		if (url.empty()) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid url");
			ErrorCode = InvalidURL;

			return{};
		}
		auto handle = url_fopen(url.c_str(), "rb");
		if (!handle) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open url: %s", url.c_str()).c_str());
			ErrorCode = CannotConnect;

			return{};
		}
		
		char buffer[256];
		int nread = 0;
		ByteArray ret;
		do {
			nread = url_fread(buffer, 1, sizeof(buffer), handle);
			if (nread)
				ret.insert(ret.end(), buffer, buffer + nread);
		} while (nread);
		contentType = handle->contentType;
		url_fclose(handle);
		return ret;
	}

	struct HostInfo {
		time_t mTryAgainInterval;
		int mAttemptLimit;
		std::atomic<int> mLogCount;
		std::atomic<time_t> mLastLogTime;

		HostInfo() {}
		HostInfo(int attemptLimit, time_t tryAgainInterval)
			: mTryAgainInterval(tryAgainInterval)
			, mAttemptLimit(attemptLimit)
		{
			mLastLogTime = gpTimer->GetTickCount();
			mLogCount = 1;
		}

		bool IsAvailable()
		{
			return mLogCount < mAttemptLimit;
		}

		bool IsTimeToTryAgain()
		{
			return gpTimer->GetTickCount() - mLastLogTime >= mTryAgainInterval;
		}

		HostInfo& operator=(const HostInfo& other) {
			mTryAgainInterval = other.mTryAgainInterval;
			mAttemptLimit = other.mAttemptLimit;
			mLogCount = other.mLogCount.load(std::memory_order_relaxed);
			mLastLogTime = other.mLastLogTime.load(std::memory_order_relaxed);
			return *this;
		}
	};

	std::map<std::string, HostInfo> sHostMap;
	bool Network::isHostAvailable(const char* host) {
		if (!ValidCStringLength(host))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}

		auto hostName = GetURIHost(host);
		auto it = sHostMap.find(hostName);
		if (it == sHostMap.end())
			return true;

		HostInfo& hi = it->second;
		if (hi.IsTimeToTryAgain())
		{
			hi.mLogCount = 0; // info removed from table in logAvailableHost
			return true;
		}

		return hi.IsAvailable();
	}

	void Network::logUnavailableHost(const char* url) {
		auto hostName = GetURIHost(url);
		auto it = sHostMap.find(hostName);
		if (it != sHostMap.end()) {
			auto& hi = it->second;
			if (hi.IsAvailable()) {
				hi.mLogCount.fetch_add(1);
			}
			hi.mLastLogTime = gpTimer->GetTickCount();
		}
		else {
			HostInfo hi(10, 2000);
			sHostMap[hostName] = hi;
		}
	}

	void Network::logAvailableHost(const char* url) {
		auto hostName = GetURIHost(url);
		auto it = sHostMap.find(hostName);
		if (it != sHostMap.end()) 
		{
			sHostMap.erase(it);
		}
	}

	std::string Network::GetURIPath(const std::string& url) {
		boost::network::uri::uri uri(url);
		auto p = uri.path();
		if (p.empty())
			return url;
		return p;
	}


	std::string Network::GetURIString(const std::string& url) {
		boost::network::uri::uri uri(url);
		return uri.string();
	}

	std::string Network::GetURIHost(const std::string& url) {
		boost::network::uri::uri uri(url);
		return uri.host();
	}

	std::string Network::GetURIQuery(const std::string& url) {
		boost::network::uri::uri uri(url);
		return uri.query();
	}

	std::string Network::GetURIProtocol(const std::string& url) {
		boost::network::uri::uri uri(url);
		return uri.scheme();
	}

	void Network::GetURIHostPathQuery(const std::string& url,
		std::string& host, std::string& path, std::string& query) {
		boost::network::uri::uri uri(url);
		host = uri.host();
		path = uri.path();
		query = uri.query();
	}

	void Network::GetURIHostPath(const std::string& url,
		std::string& host, std::string& path) {
		boost::network::uri::uri uri(url);
		host = uri.host();
		path = uri.path();
	}

	void Network::GetURIAuthorityPath(const std::string& url,
		std::string& authority, std::string& path)
	{
		boost::network::uri::uri uri(url);
		authority = boost::network::uri::authority(uri);
		path = uri.path();
	}
}
