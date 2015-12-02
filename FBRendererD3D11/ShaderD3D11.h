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
#ifndef _fastbird_ShaderD3D11_header_included_
#define _fastbird_ShaderD3D11_header_included_

#include "FBRenderer/IPlatformShader.h"
#include <set>
namespace fb
{
	FB_DECLARE_SMART_PTR(ShaderD3D11);
	class ShaderD3D11 : public IPlatformShader
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(ShaderD3D11);
		ShaderD3D11();

	public:
		typedef std::set<std::string> INCLUDE_FILES;

		static ShaderD3D11Ptr Create();		

		//---------------------------------------------------------------------------
		// IPlatformShader
		//---------------------------------------------------------------------------
		/**Bind all type of platform shaders
		Empty shader will be removed from the pipeline.
		*/
		void Bind();
		/** Bind vertex shader only.*/
		void BindVS();
		/** Bind hull shader only.*/
		void BindHS();
		/** Bind domain shader only.*/
		void BindDS();
		/** Bind geometry shader only.*/
		void BindGS();
		/** Bind pixel shader only.	*/
		void BindPS();		
		
		bool GetCompileFailed() const;
		void* GetVSByteCode(unsigned& size) const;				
		void SetDebugName(const char* name);
		/** Returns true if \a inc is a related header file.
		*/
		bool CheckIncludes(const char* inc);
		
		//---------------------------------------------------------------------------
		// Own
		//---------------------------------------------------------------------------
		void SetVertexShader(ID3D11VertexShader* pVertexShader);
		void SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode);
		void SetVertexShaderBytecode(void* bytecode, size_t size);
		void SetGeometryShader(ID3D11GeometryShader* pGeometryShader);
		void SetHullShader(ID3D11HullShader* pHullShader);
		void SetDomainShader(ID3D11DomainShader* pDomainShader);
		void SetPixelShader(ID3D11PixelShader* pPixelShader);		

		ID3D11VertexShader* GetVertexShader() const;
		ID3D11GeometryShader* GetGeometryShader() const;
		ID3D11HullShader* GetHullShader() const;
		ID3D11DomainShader* GetDomainShader() const;
		ID3D11PixelShader* GetPixelShader() const;
	
		void SetIncludeFiles(const INCLUDE_FILES& ifs);
		void SetCompileFailed(bool failed);
	};
}

#endif //_fastbird_ShaderD3D11_header_included_