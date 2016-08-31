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

	template<class T>
	class Buffer {
		std::vector<T> mData;
		unsigned mPosition;

	public:
		Buffer()
			:mPosition(0)
		{
		}

		unsigned Position() const {
			return mPosition;
		}

		unsigned Limit() const {
			return mData.size();
		}

		unsigned Size() const {
			return mData.size();
		}

		T& operator[](unsigned index) {
			if (index >= mData.size()) {
				throw std::exception("Out of index");
			}
			return mData[index];
		}

		T Get() {
			if (mPosition >= mData.size()) {
				return T();
			}

			return mData[mPosition++];
		}

		Buffer<T>& put(const T& t) {
			mData.insert(mData.begin() + mPosition, t);
			++mPosition;
			return *this;
		}
	};

	typedef Buffer<int> IntBuffer;
	typedef Buffer<short> ShortBuffer;
	typedef Buffer<Real> RealBuffer;

}
