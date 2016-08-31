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

// fbd.cpp : Defines the entry point for the console application.
//
#include "fba.h"
#include "fba_header.h"
#include "compress_uncompress.h"
#include "zlib.h"

namespace fb {
	int compress(std::vector<char>& compressed_data, const std::vector<char>& original_data) {
		auto deflated_size = compressBound(original_data.size());
		compressed_data.resize(deflated_size);
		auto ret = ::compress((Bytef*)&compressed_data[0], &deflated_size, (Bytef*)&original_data[0], original_data.size());
		compressed_data.resize(deflated_size);
		return ret;
	}

	int uncompress(char* uncompressed_data, uLongf uncompressed_size, const std::vector<char>& compressed_data) {		
		auto uncompressed_size2 = uncompressed_size;
		auto ret = ::uncompress((Bytef*)&uncompressed_data[0], &uncompressed_size, (Bytef*)&compressed_data[0], compressed_data.size());
		if (uncompressed_size != uncompressed_size2) {
			std::cerr << "The size of uncompressed_data is not the expected size.";
			return Z_DATA_ERROR;
		}
		return ret;
	}

	int uncompress(std::vector<char>& uncompressed_data, const std::vector<char>& compressed_data) {
		if (uncompressed_data.empty()) {
			std::cerr << "inflated_data should have enough memory for holding uncompressed data before calling this function." << std::endl;
			return Z_MEM_ERROR;
		}		
		return uncompress(&uncompressed_data[0], uncompressed_data.size(), compressed_data);
	}

	int uncompress(std::vector<unsigned char>& uncompressed_data, const std::vector<char>& compressed_data) {
		if (uncompressed_data.empty()) {
			std::cerr << "inflated_data should have enough memory for holding uncompressed data before calling this function." << std::endl;
			return Z_MEM_ERROR;
		}
		return uncompress((char*)&uncompressed_data[0], uncompressed_data.size(), compressed_data);
	}
}
