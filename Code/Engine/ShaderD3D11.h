#pragma once
#ifndef _fastbird_ShaderD3D11_header_included_
#define _fastbird_ShaderD3D11_header_included_

#include <Engine/Shader.h>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace fastbird
{
	class ShaderD3D11 : public Shader
	{
	public:
		static ShaderD3D11* CreateInstance(const char* name);
		virtual void Delete();

		virtual ~ShaderD3D11();
		
		void SetVertexShader(ID3D11VertexShader* pVertexShader);
		void SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode);
		void SetVertexShaderBytecode(void* bytecode, size_t size);
		void SetGeometryShader(ID3D11GeometryShader* pGeometryShader);
		void SetHullShader(ID3D11HullShader* pHullShader);
		void SetDomainShader(ID3D11DomainShader* pDomainShader);
		void SetPixelShader(ID3D11PixelShader* pPixelShader);
		virtual void Bind();
		virtual void BindVS();
		virtual void BindGS();
		virtual void BindPS();
		virtual void BindDS();
		virtual void BindHS();
		virtual void* GetVSByteCode(unsigned& size) const;

		ID3D11VertexShader* GetVertexShader() const { return m_pVertexShader; }
		ID3D11GeometryShader* GetGeometryShader() const { return m_pGeometryShader; }
		ID3D11HullShader* GetHullShader() const { return m_pHullShader; }
		ID3D11DomainShader* GetDomainShader() const { return m_pDomainShader; }
		ID3D11PixelShader* GetPixelShader() const { return m_pPixelShader; }
		bool IsValid() const;
		virtual void SetDebugName(const char*);


	protected:
		ShaderD3D11( const char* name );

	private:
		ID3D11VertexShader* m_pVertexShader;
		ID3D11GeometryShader* m_pGeometryShader;
		ID3D11HullShader* m_pHullShader;
		ID3D11DomainShader* m_pDomainShader;
		ID3D11PixelShader* m_pPixelShader;
		bool mValid;
		BinaryData mVSBytecode;
		size_t mBytecodeSize;
	};
}

#endif //_fastbird_ShaderD3D11_header_included_