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
#include <memory>
namespace fb {
	// T assumed to be the class T of shared_ptr
	template <class T>
	class QuadNode
	{
	protected:
		using TPtr = std::shared_ptr<T>;
		using TWeakPtr = std::weak_ptr<T>;
		TPtr mQuadTreeChild[4];
		TWeakPtr mQuadTreeParent;

	public:
		bool HasChildrenNodes() const {
			return mQuadTreeChild[0] != nullptr;
		}
		bool HasParentNode() const {
			return !mQuadTreeParent.expired()
		}

		TPtr GetParent() const {
			return mQuadTreeParent;
		}

		void SetChildrenNodes(const std::vector<TPtr>& children) {
			if (children.size() != 4) {
				assert(0 && "Invalid children size");
				return;
			}
			for (int i = 0; i < 4; ++i) {
				mQuadTreeChild[i] = children[i];				
			}
		}

		void SetParentNode(TPtr parent) {
			mQuadTreeParent = parent;
		}

		std::vector<TPtr> GetChildrenNodes() const {
			return{ mQuadTreeChild[0], mQuadTreeChild[1] , mQuadTreeChild[2] , mQuadTreeChild[3] };
		}
	};
}
