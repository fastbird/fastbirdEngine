#pragma once

#include "SmartPtr.h"

/* for ini - from http://code.google.com/p/inih/
The "inih" library is distributed under the New BSD license:

	Copyright (c) 2009, Brush Technology
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of Brush Technology nor the names of its contributors
		  may be used to endorse or promote products derived from this software
		  without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY BRUSH TECHNOLOGY ''AS IS'' AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL BRUSH TECHNOLOGY BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/


namespace fastbird
{
	//------------------------------------------------------------------------
	class INIReader : public ReferenceCounter
	{
	public:
		INIReader(const char* filename);
	
		// return 0 if succeed
		int GetError();

		std::string Get(const char* section, const char* name,
						const char* defaultValue);

		long GetInteger(const char* section, const char* name, long default_value);
		bool GetBoolean(const char* section, const char* name, bool default_value);

	private:
		int mError;
		std::map<std::string, std::string> mValues;

		static std::string MakeKey(const char* section, const char* name);
		static int ValueHandler(void* user, const char* section, const char* name,
								const char* value);
	};

	//------------------------------------------------------------------------
	int ini_parse(const char* filename,
              int (*handler)(void* user, const char* section,
                             const char* name, const char* value),
              void* user);

	//------------------------------------------------------------------------
	int ini_parse_file(FILE* file,
                   int (*handler)(void* user, const char* section,
                                  const char* name, const char* value),
                   void* user);
}
