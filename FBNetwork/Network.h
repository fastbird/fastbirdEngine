#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBNetwork/NetworkExceptions.h"
namespace fb {
	class FB_DLL_NETWORK Network
	{
	public:
		static int ErrorCode;
		enum
		{
			NoError,
			CannotConnect,
			InvalidURL,
		};
		// curl_global_init
		// should call these initialize function in application side.
		static void Initialize();
		static void Uninitialize();

		static std::string MakeMimeTypeForSuffix(const char* suffix);
		/// . is included
		static std::string MakeSuffixForMimeType(const char* mimeType);
		static ByteArray readURLContentToBuffer(const std::string& url);
		static ByteArray readURLContentToBuffer(const std::string& url, std::string& contentType);
		static ByteArray readURLContentToBuffer(const std::string& url, std::string& contentType, bool allocateDirect);
		static bool isHostAvailable(const char* url);
		static void logUnavailableHost(const char* url);
		static void logAvailableHost(const char* url);

		static std::string GetURIPath(const std::string& url);
		static std::string GetURIString(const std::string& url);
		static std::string GetURIHost(const std::string& url);
		static std::string GetURIQuery(const std::string& url);
		static std::string GetURIProtocol(const std::string& url);
		static void GetURIHostPathQuery(const std::string& url,
			std::string& host, std::string& path, std::string& query);
		static void GetURIHostPath(const std::string& url,
			std::string& host, std::string& path);
		static void GetURIAuthorityPath(const std::string& url,
			std::string& authority, std::string& path);		
	};
}