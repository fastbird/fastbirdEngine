#pragma once
#ifndef _fastbird_ShaderD3D11_header_included_
#define _fastbird_ShaderD3D11_header_included_

#include <Engine/Renderer/Shader.h>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace fastbird
{
	class ShaderD3D11 : public Shader
	{
	public:
		static ShaderD3D11* CreateInstance(const char* name);
		
		void SetVertexShader(ID3D11VertexShader* pVertexShader);
		void SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode);
		void SetGeometryShader(ID3D11GeometryShader* pGeometryShader);
		void SetHullShader(ID3D11HullShader* pHullShader);
		void SetDomainShader(ID3D11DomainShader* pDomainShader);
		void SetPixelShader(ID3D11PixelShader* pPixelShader);
		virtual void Bind();
		virtual void* GetVSByteCode(unsigned& size) const;

		ID3D11VertexShader* GetVertexShader() const { return m_pVertexShader; }
		ID3D11GeometryShader* GetGeometryShader() const { return m_pGeometryShader; }
		ID3D11HullShader* GetHullShader() const { return m_pHullShader; }
		ID3D11DomainShader* GetDomainShader() const { return m_pDomainShader; }
		ID3D11PixelShader* GetPixelShader() const { return m_pPixelShader; }
		bool IsValid() const;


	protected:
		ShaderD3D11( const char* name );
		virtual ~ShaderD3D11();

	private:
		ID3D11VertexShader* m_pVertexShader;
		ID3DBlob* m_pVertexShaderBytecode;
		ID3D11GeometryShader* m_pGeometryShader;
		ID3D11HullShader* m_pHullShader;
		ID3D11DomainShader* m_pDomainShader;
		ID3D11PixelShader* m_pPixelShader;
	};
}

#endif //_fastbird_ShaderD3D11_header_included_