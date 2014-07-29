#include "Engine/StdAfx.h"
#include <Engine/Renderer/D3D11/ShaderD3D11.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/Renderer/D3D11/RendererD3D11.h>

using namespace fastbird;

ShaderD3D11* ShaderD3D11::CreateInstance(const char* name)
{
	ShaderD3D11* pShader = new ShaderD3D11(name);
	return pShader;
}

ShaderD3D11::ShaderD3D11( const char* name )
	: m_pVertexShader(0), m_pGeometryShader(0), m_pHullShader(0)
	, m_pDomainShader(0), m_pPixelShader(0)
	, m_pVertexShaderBytecode(0)
	, Shader(name)
{

}

ShaderD3D11::~ShaderD3D11()
{
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pGeometryShader);
	SAFE_RELEASE(m_pHullShader);
	SAFE_RELEASE(m_pDomainShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexShaderBytecode);
}

void ShaderD3D11::SetVertexShader(ID3D11VertexShader* pVertexShader)
{
	SAFE_RELEASE(m_pVertexShader);
	m_pVertexShader = pVertexShader;
}

void ShaderD3D11::SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode)
{
	SAFE_RELEASE(m_pVertexShaderBytecode);
	m_pVertexShaderBytecode = pVertexShaderBytecode;
}

void ShaderD3D11::SetGeometryShader(ID3D11GeometryShader* pGeometryShader)
{
	SAFE_RELEASE(m_pGeometryShader);
	m_pGeometryShader = pGeometryShader;
}

void ShaderD3D11::SetHullShader(ID3D11HullShader* pHullShader)
{
	SAFE_RELEASE(m_pHullShader);
	m_pHullShader = pHullShader;
}

void ShaderD3D11::SetDomainShader(ID3D11DomainShader* pDomainShader)
{
	SAFE_RELEASE(m_pDomainShader);
	m_pDomainShader = pDomainShader;
}

void ShaderD3D11::SetPixelShader(ID3D11PixelShader* pPixelShader)
{
	SAFE_RELEASE(m_pPixelShader);
	m_pPixelShader = pPixelShader;
}

//----------------------------------------------------------------------------
void ShaderD3D11::Bind()
{
	gFBEnv->pEngine->GetRenderer()->SetShader(this);
}

//----------------------------------------------------------------------------
void* ShaderD3D11::GetVSByteCode(unsigned& size) const
{
	if (m_pVertexShaderBytecode)
	{
		size = m_pVertexShaderBytecode->GetBufferSize();
		return m_pVertexShaderBytecode->GetBufferPointer();
	}

	size = 0;
	return 0;
}

bool ShaderD3D11::IsValid() const
{
	return m_pVertexShader!=0;
}