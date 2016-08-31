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
#include "RendererEnums.h"
#include "ShaderDefines.h"
namespace fb {
	struct PlatformShaderKey {
	private:
		SHADER_TYPE mShader;
		std::string mFilepath;
		SHADER_DEFINES mDefines;
		size_t mHash;

	public:
		PlatformShaderKey(const char* path, SHADER_TYPE shader, const SHADER_DEFINES& defines)
			: mFilepath(path)
			, mShader(shader)
			, mDefines(defines)
		{
			std::sort(mDefines.begin(), mDefines.end());
			mHash = std::hash<SHADER_TYPE>()(mShader);
			hash_combine(mHash, std::hash<std::string>()(mFilepath));
			hash_combine(mHash, fb::Hash(mDefines));
		}

		size_t Hash() const {
			return mHash;
		}

		SHADER_TYPE GetShaderType() const {
			return mShader;
		}
		const char* GetFilePath() const {
			return mFilepath.c_str();
		}

		const SHADER_DEFINES& GetShaderDefines() const {
			return mDefines;
		}

		bool operator < (const PlatformShaderKey& other) const {
			return mHash < other.mHash;
		}

		bool operator == (const PlatformShaderKey& other) const {
			return mHash == other.mHash;
		}
	};

	struct ShaderKey {
		int mShaderTypes;
		std::string mFilepath;
		SHADER_DEFINES mDefines;
		size_t mHash;

		ShaderKey(const char* path, int shaderTypes, const SHADER_DEFINES& defines)
			: mFilepath(path)
			, mShaderTypes(shaderTypes)
			, mDefines(defines)
		{
			std::sort(mDefines.begin(), mDefines.end());

			mHash = std::hash<int>()(mShaderTypes);
			hash_combine(mHash, std::hash<std::string>()(mFilepath));
			hash_combine(mHash, fb::Hash(mDefines));
		}

		size_t Hash() const {
			return mHash;
		}

		bool operator < (const ShaderKey& other) const {
			return mHash < other.mHash;
		}

		bool operator == (const ShaderKey& other) const {
			return mHash == other.mHash;
		}
	};
}


namespace std {
	template<>
	struct hash<fb::PlatformShaderKey>
		: public _Bitwise_hash<fb::PlatformShaderKey>
	{
		typedef fb::PlatformShaderKey _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};

	template<>
	struct hash<fb::ShaderKey>
		: public _Bitwise_hash<fb::ShaderKey>
	{
		typedef fb::ShaderKey _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};
}