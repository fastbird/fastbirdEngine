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

#include <iostream>
#include "Serialization.h"
namespace fb{
	void write(std::ostream& stream, const std::string& str){
		size_t size = str.size();
		unsigned size2 = (unsigned)size;
		if (size != size2) {
			std::cerr << "Maximum size exeeded." << std::endl;
		}
		stream.write((char*)&size2, sizeof(size2));
		if (size2 > 0){
			stream.write(&str[0], size2);
		}
	}
	void read(std::istream& stream, std::string& str){
		unsigned size;
		stream.read((char*)&size, sizeof(size));
		if (size > 0){
			str.assign((size_t)size, (char)0);
			stream.read(&str[0], size);
		}
	}
}