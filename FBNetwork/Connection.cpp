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

const ByteArray& Connection::GetStringBuffer() {
	if (mFile && mBuffer.empty()) {
		char buffer[256];
		int nread = 0;
		do {
			nread = url_fread(buffer, 1, sizeof(buffer), mFile);
			if (nread)
				mBuffer.insert(mBuffer.end(), buffer, buffer + nread);
		} while (nread);
	}
	if (mBuffer.back() != 0) {
		mBuffer.push_back(0);
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