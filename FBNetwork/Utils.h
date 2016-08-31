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
namespace fb {	
	typedef void CURL;

	enum fcurl_type_e {
		CFTYPE_NONE = 0,
		CFTYPE_FILE = 1,
		CFTYPE_CURL = 2
	};

	struct fcurl_data
	{
		enum fcurl_type_e type;     /* type of handle */
		union {
			CURL *curl;
			FILE *file;
		} handle;                   /* handle */

		char *buffer;               /* buffer to store cached data*/
		size_t buffer_len;          /* currently allocated buffers length */
		size_t buffer_pos;          /* end of data in buffer*/
		int still_running;          /* Is background url fetch still in progress */
		char contentType[255];
	};

	typedef struct fcurl_data URL_FILE;

	void initializeCURL();
	void uninitializeCURL();
	URL_FILE *url_fopen(const char *url, const char *operation);
	void url_fclose(URL_FILE *file);
	size_t header_callback(char *buffer, size_t size,
		size_t nitems, void *userdata);
	size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp);
	int fill_buffer(URL_FILE *file, size_t want);
	int use_buffer(URL_FILE *file, size_t want);
	int url_feof(URL_FILE *file);
	size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
	char *url_fgets(char *ptr, size_t size, URL_FILE *file);
	void url_rewind(URL_FILE *file);
}
