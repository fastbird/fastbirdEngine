#include "stdafx.h"
#include "Connection.h"
#include "Network.h"
#include "Utils.h"
using namespace fb;

Connection::Connection(const std::string& url) {
	mFile = url_fopen(url.c_str(), "r");
	if (!mFile) {
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Cannot open url: %s", url.c_str()).c_str());		
	}
}

Connection::~Connection()
{
	url_fclose(mFile);
}

void Connection::SetConnectTimeout(time_t t) {

}

void Connection::SetReadTimeout(time_t t) {

}

size_t Connection::GetContentLength() {
	return mBuffer.size();
}

const ByteArray& Connection::GetBuffer() {
	if (mFile && mBuffer.empty()) {
		char buffer[256];
		int nread = 0;		
		do {
			nread = url_fread(buffer, 1, sizeof(buffer), mFile);
			if (nread)
				mBuffer.insert(mBuffer.end(), buffer, buffer + nread);
		} while (nread);		
	}
	return mBuffer;
}

std::string Connection::getHeaderField(const char* header) const {
	return{};
}

int Connection::getResponseCode() const {
	return 0;
}

std::string Connection::getResponseMessage() const {
	return{};
}

const char* Connection::getContentType() const {
	if (!mFile)
		return "";

	return mFile->contentType;
}