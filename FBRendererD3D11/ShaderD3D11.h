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
	protected:
		ShaderD3D11();

	public:
		//---------------------------------------------------------------------------
		// IPlatformShader
		//---------------------------------------------------------------------------		
		bool GetCompileFailed() const OVERRIDE;
		virtual void* GetVSByteCode(unsigned& size) const OVERRIDE;		
		virtual bool RunComputeShader(void* constants, size_t size,
			int x, int y, int z, ByteArray& output, size_t outputSize) OVERRIDE;
		virtual bool QueueRunComputeShader(void* constants, size_t size,
			int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&&) OVERRIDE;
		bool IsRelatedFile(const char* file) OVERRIDE;		
		bool Reload(const SHADER_DEFINES& defines) OVERRIDE;
		
		//---------------------------------------------------------------------------
		// Own
		//---------------------------------------------------------------------------
		// the first is the main hlsl file and others are include files.
		void SetRelatedFiles(const StringVector& files);
		void SetCompileFailed(bool failed);	
		const StringVector& GetRelatedFiles() const;
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(VertexShaderD3D11);
	class VertexShaderD3D11 : public ShaderD3D11 {
		FB_DECLARE_PIMPL_NON_COPYABLE(VertexShaderD3D11);
		VertexShaderD3D11();

	public:		
		static VertexShaderD3D11Ptr Create();		
		void Bind() OVERRIDE;		
		void SetDebugName(const char* name) OVERRIDE;
		SHADER_TYPE GetShaderType() const OVERRIDE;
		void* GetVSByteCode(unsigned& size) const OVERRIDE;		

		void SetVertexShader(ID3D11VertexShader* pVertexShader);
		void SetVertexShaderBytecode(ByteArray&& bytecode);
		void SetVertexShaderBytecode(void* bytecode, size_t size);
		ID3D11VertexShader* GetVertexShader() const;
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(GeometryShaderD3D11);
	class GeometryShaderD3D11 : public ShaderD3D11 {
		FB_DECLARE_PIMPL_NON_COPYABLE(GeometryShaderD3D11);
		GeometryShaderD3D11();

	public:
		static GeometryShaderD3D11Ptr Create();
		void Bind() OVERRIDE;
		void SetDebugName(const char* name) OVERRIDE;		
		SHADER_TYPE GetShaderType() const OVERRIDE;

		void SetGeometryShader(ID3D11GeometryShader* pGeometryShader);
		ID3D11GeometryShader* GetGeometryShader() const;
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(PixelShaderD3D11);
	class PixelShaderD3D11 : public ShaderD3D11 {
		FB_DECLARE_PIMPL_NON_COPYABLE(PixelShaderD3D11);
		PixelShaderD3D11();

	public:
		static PixelShaderD3D11Ptr Create();
		void Bind() OVERRIDE;
		void SetDebugName(const char* name) OVERRIDE;		
		SHADER_TYPE GetShaderType() const OVERRIDE;

		void SetPixelShader(ID3D11PixelShader* pPixelShader);
		ID3D11PixelShader* GetPixelShader() const;
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(ComputeShaderD3D11);
	class ComputeShaderD3D11 : public ShaderD3D11 {
		FB_DECLARE_PIMPL_NON_COPYABLE(ComputeShaderD3D11);
		ComputeShaderD3D11();

	public:
		static ComputeShaderD3D11Ptr Create();
		void Bind() OVERRIDE;
		void SetDebugName(const char* name) OVERRIDE;
		SHADER_TYPE GetShaderType() const OVERRIDE;
		bool RunComputeShader(void* constants, size_t size,
			int x, int y, int z, ByteArray& output, size_t outputSize) OVERRIDE;
		bool QueueRunComputeShader(void* constants, size_t size,
			int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&& callback) OVERRIDE;

		void SetComputeShader(ID3D11ComputeShader* pComputeShader);
		ID3D11ComputeShader* GetComputeShader() const;
	};
}

#endif //_fastbird_ShaderD3D11_header_included_