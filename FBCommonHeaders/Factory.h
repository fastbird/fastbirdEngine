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
#include <map>
#include <string>
#include <iostream>
#include <memory>
namespace fb{
	template <typename T>
	class Factory{
	public:
		typedef std::shared_ptr<T> (*CreateCallback)();
		bool RegisterProduct(const char* type, CreateCallback callback){
			if (type == 0 || strlen(type) == 0 || callback == 0){
				std::cerr << "Factory:RegisterProduct : Invalid arguments\n";
				return false;
			}
			if (mProducts.find(type) == mProducts.end()){
				mProducts[std::string(type)] = callback;
				return true;
			}
			std::cerr << "Already registered product type : " << type << std::endl;
			return false;
		}
		void UnregisterProduct(const char* type){
			if (type == 0 || strlen(type) == 0){
				std::cerr << "Factory:RegisterProduct : Invalid arguments\n";
				return;
			}
			auto it = mProducts.find(std::string(type));
			if (it != mProducts.end()){
				mProducts.erase(it);
			}
		}
		std::shared_ptr<T> CreateProduct(const char* type){
			if (type == 0 || strlen(type) == 0){
				std::cerr << "Factory:CreateProduct : Invalid arguments\n";
				return 0;
			}
			auto it = mProducts.find(type);
			if (it != mProducts.end()){
				return it->second();
			}
			std::cerr << "Factory:CreateProduct : Not registered type.\n";
			return 0;
		}

	private:
		typedef std::map<std::string, CreateCallback> Products;
		Products mProducts;
	};
}