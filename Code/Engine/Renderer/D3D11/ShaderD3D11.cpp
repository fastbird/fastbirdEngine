#include "Engine/StdAfx.h"
#include <Engine/Renderer/D3D11/ShaderD3D11.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/Renderer/D3D11/RendererD3D11.h>

using namespace fastbird;

ShaderD3D11* ShaderD3D11::CreateInstance(const char* name)
{
	ShaderD3D11* pShader = FB_NEW(ShaderD3D11)(name);
	return pShader;
}

void ShaderD3D11::Delete()
{
	FB_DELETE(this);
}

ShaderD3D11::ShaderD3D11( const char* name )
	: m_pVertexShader(0), m_pGeometryShader(0), m_pHullShader(0)
	, m_pDomainShader(0), m_pPixelShader(0)
	, mVSBytecode(0)
	, mBytecodeSize(0)
	, mValid(false)
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
	if (mVSBytecode)
	{
		FB_SAFE_ARRDEL(mVSBytecode);
	}
}

void ShaderD3D11::SetVertexShader(ID3D11VertexShader* pVertexShader)
{
	SAFE_RELEASE(m_pVertexShader);
	m_pVertexShader = pVertexShader;
	mValid = true;
}

void ShaderD3D11::SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode)
{
	if (mVSBytecode)
	{
		FB_SAFE_ARRDEL(mVSBytecode);
	}
	if (pVertexShaderBytecode)
	{
		mBytecodeSize = pVertexShaderBytecode->GetBufferSize();
		mVSBytecode = FB_ARRNEW(char, mBytecodeSize);
		memcpy(mVSBytecode, pVertexShaderBytecode->GetBufferPointer(), mBytecodeSize);
	}
}

void ShaderD3D11::SetVertexShaderBytecode(void* bytecode, size_t size)
{
	if (mVSBytecode)
	{
		FB_SAFE_ARRDEL(mVSBytecode);
	}
	if (bytecode)
	{
		mBytecodeSize = size;
		mVSBytecode = FB_ARRNEW(char, mBytecodeSize);
		memcpy(mVSBytecode, bytecode, mBytecodeSize);
	}
}

void ShaderD3D11::SetGeometryShader(ID3D11GeometryShader* pGeometryShader)
{
	SAFE_RELEASE(m_pGeometryShader);
	m_pGeometryShader = pGeometryShader;
	mValid = true;
}

void ShaderD3D11::SetHullShader(ID3D11HullShader* pHullShader)
{
	SAFE_RELEASE(m_pHullShader);
	m_pHullShader = pHullShader;
	mValid = true;
}

void ShaderD3D11::SetDomainShader(ID3D11DomainShader* pDomainShader)
{
	SAFE_RELEASE(m_pDomainShader);
	m_pDomainShader = pDomainShader;
	mValid = true;
}

void ShaderD3D11::SetPixelShader(ID3D11PixelShader* pPixelShader)
{
	SAFE_RELEASE(m_pPixelShader);
	m_pPixelShader = pPixelShader;
	mValid = true;
}

//----------------------------------------------------------------------------
void ShaderD3D11::Bind()
{
	gFBEnv->pEngine->GetRenderer()->SetShaders(this);
}

void ShaderD3D11::BindVS()
{
	gFBEnv->pEngine->GetRenderer()->SetVSShader(this);
}
void ShaderD3D11::BindGS()
{
	gFBEnv->pEngine->GetRenderer()->SetGSShader(this);
}
void ShaderD3D11::BindPS()
{
	gFBEnv->pEngine->GetRenderer()->SetPSShader(this);
}
void ShaderD3D11::BindDS()
{
	gFBEnv->pEngine->GetRenderer()->SetDSShader(this);
}
void ShaderD3D11::BindHS()
{
	gFBEnv->pEngine->GetRenderer()->SetHSShader(this);
}

//----------------------------------------------------------------------------
void* ShaderD3D11::GetVSByteCode(unsigned& size) const
{
	if (mVSBytecode)
	{
		size = mBytecodeSize;
		return mVSBytecode;
	}

	size = 0;
	return 0;
}

bool ShaderD3D11::IsValid() const
{
	return mValid;
}

//----------------------------------------------------------------------------
void ShaderD3D11::SetDebugName(const char*)
{
	char buf[256];
	if (m_pVertexShader)
	{
		sprintf_s(buf, "%s VS");
		m_pVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}

	if (m_pGeometryShader)
	{
		sprintf_s(buf, "%s GS");
		m_pGeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}

	if (m_pHullShader)
	{
		sprintf_s(buf, "%s HS");
		m_pHullShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}

	if (m_pDomainShader)
	{
		sprintf_s(buf, "%s DS");
		m_pDomainShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}

	if (m_pPixelShader)
	{
		sprintf_s(buf, "%s PS");
		m_pPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}
		
}