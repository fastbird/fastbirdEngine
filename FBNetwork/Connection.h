#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBNetwork/NetworkExceptions.h"
namespace fb {
	struct fcurl_data;
	FB_DECLARE_SMART_PTR(Connection);
	class FB_DLL_NETWORK Connection {
		ByteArray mBuffer;		
		fcurl_data* mFile;
	public:
		Connection(const std::string& url);
		~Connection();
		void SetConnectTimeout(time_t t);
		void SetReadTimeout(time_t t);
		size_t GetContentLength();
		const ByteArray& GetBuffer() ;
		std::string getHeaderField(const char* header) const;
		int getResponseCode() const;
		std::string getResponseMessage() const;
		const char* getContentType() const;		
	};
}
