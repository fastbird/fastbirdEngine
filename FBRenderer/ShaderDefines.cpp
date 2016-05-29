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
#include "ShaderDefines.h"
#include "FBCommonHeaders/Helpers.h"
namespace fb{
	ShaderDefine::ShaderDefine(const char* _name, const char* _value)
		: mName(_name), mValue(_value)
	{
		mHash = std::hash<std::string>()(mName);
		hash_combine(mHash, std::hash<std::string>()(mValue));
	}

	size_t ShaderDefine::Hash() const {
		return mHash;
	}

	const std::string& ShaderDefine::GetName() const {
		return mName;
	}

	const std::string& ShaderDefine::GetValue() const {
		return mValue;
	}

	bool ShaderDefine::operator == (const ShaderDefine& b) const{
		if (mName == b.mName && mValue == b.mValue)
			return true;

		return false;
	}

	bool ShaderDefine::operator!=(const ShaderDefine& b) const{
		return !operator==(b);
	}

	bool ShaderDefine::operator< (const ShaderDefine& b) const{
		if (mName < b.mName)
			return true;
		else if (mName == b.mName){
			return mValue < b.mValue;
		}
		return false;
	}

	size_t Hash(const SHADER_DEFINES& shaderDefines) {
		size_t h = 0;
		for (auto sd : shaderDefines) {
			hash_combine(h, sd.Hash());
		}
		return h;
	}

} // namespace fb


