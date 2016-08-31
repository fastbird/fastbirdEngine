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
#include <iostream>
#include <vector>
namespace fb{
	enum {
		serialization_version = 16081900
	};
	// normal
	template <class T, typename std::enable_if_t < std::is_pod<T>::value && !std::is_array<T>::value >* = nullptr>
	void write(std::ostream& out, const T& data) {
		out.write((char*)&data, sizeof(T));
	}

	template <class T, typename std::enable_if_t < std::is_pod<T>::value && !std::is_array<T>::value >* = nullptr>
	void read(std::istream& in, T& data) {
		in.read((char*)&data, sizeof(T));
	}

	// char array
	template <size_t _Size>
	void write(std::ostream& out, char (&data)[_Size]) {
		out.write(data, _Size);
	}

	template <size_t _Size>
	void read(std::istream& in, char (&data)[_Size]) {
		in.read(data, _Size);
	}

	// Vectors - POD
	template < class T, typename std::enable_if_t< std::is_pod<T>::value >* = nullptr >
	void write(std::ostream& stream, const std::vector<T>& data)
	{
		auto size = data.size();
		unsigned save_size = (unsigned)size;
		if ((size_t)save_size != size) {
			std::cerr << "(error) Data is huge. Consider to change 8 bytes.\n";
			assert(0 && "Data is huge. Consider to change 8 bytes.");			
		}
		write(stream, save_size);
		for (const auto& d : data){
			write(stream, d);
		}
	}

	template < class T, typename std::enable_if_t < std::is_pod<T>::value >* = nullptr >
		void read(std::istream& stream, std::vector<T>& data)
	{
		unsigned size;
		read(stream, size);		
		data.clear();
		data.reserve(size);
		for (unsigned i = 0; i < size; ++i){
			data.push_back(T());
			auto& dest = data.back();
			read(stream, dest);
		}
	}

	// Vectors - class
	template < class T, typename std::enable_if_t < !std::is_pod<T>::value >* = nullptr >
		void write(std::ostream& stream, const std::vector<T>& data, int version = serialization_version)
	{
		auto size = data.size();
		unsigned save_size = (unsigned)size;
		if ((size_t)save_size != size) {
			std::cerr << "(error) Data is huge. Consider to change 8 bytes.\n";
			assert(0 && "Data is huge. Consider to change 8 bytes.");
		}
		write(stream, save_size);
		for (const auto& d : data) {
			write(stream, d);
		}
	}

	template < class T, typename std::enable_if_t < !std::is_pod<T>::value >* = nullptr >
	void read(std::istream& stream, std::vector<T>& data, int version = 0)
	{
		unsigned size;
		read(stream, size);
		data.clear();
		data.reserve(size);
		for (unsigned i = 0; i < size; ++i) {
			data.push_back(T());
			auto& dest = data.back();
			read(stream, dest);
		}
	}

	//// Vectors - class
	//template < class T, typename std::enable_if_t < !std::is_pod<T>::value >* = nullptr >>
	//	void write_template(std::ostream& stream, const std::vector<T>& data, int version = serialization_version)
	//{
	//	auto size = data.size();
	//	unsigned save_size = (unsigned)size;
	//	if ((size_t)save_size != size) {
	//		std::cerr << "(error) Data is huge. Consider to change 8 bytes.\n";
	//		assert(0 && "Data is huge. Consider to change 8 bytes.");
	//	}
	//	write(stream, save_size);
	//	for (const auto& d : data) {
	//		write_template(stream, d);
	//	}
	//}

	//template < class T, typename std::enable_if_t < !std::is_pod<T>::value >* = nullptr >>
	//void read_template(std::istream& stream, std::vector<T>& data, int version = serialization_version)
	//{
	//	unsigned size;
	//	read(stream, size);
	//	data.clear();
	//	data.reserve(size);
	//	for (unsigned i = 0; i < size; ++i) {
	//		data.push_back(T());
	//		auto& dest = data.back();
	//		read_template(stream, dest);
	//	}
	//}

	//template < class T >
	//	void write_template(std::ostream& stream, const std::vector<std::shared_ptr<T>>& data, int version = serialization_version)
	//{
	//	auto size = data.size();
	//	unsigned save_size = (unsigned)size;
	//	if ((size_t)save_size != size) {
	//		std::cerr << "(error) Data is huge. Consider to change 8 bytes.\n";
	//		assert(0 && "Data is huge. Consider to change 8 bytes.");
	//	}
	//	write(stream, save_size);
	//	for (const auto& d : data) {
	//		write_template(stream, *d, version);
	//	}
	//}

	//template < class T >
	//void read_template(std::istream& stream, std::vector<std::shared_ptr<T>>& data, int version = serialization_version)
	//{
	//	unsigned size;
	//	read(stream, size);
	//	data.clear();
	//	data.reserve(size);
	//	for (unsigned i = 0; i < size; ++i) {
	//		data.push_back(T());
	//		auto& dest = data.back();
	//		read_template(stream, dest);
	//	}
	//}

	// string
	void write(std::ostream& stream, const std::string& str);
	void read(std::istream& stream, std::string& str);
}