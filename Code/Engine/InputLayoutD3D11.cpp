#include <Engine/StdAfx.h>
#include <Engine/InputLayoutD3D11.h>
#include <Engine/IEngine.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>

using namespace fastbird;

//----------------------------------------------------------------------------
InputLayoutD3D11* InputLayoutD3D11::CreateInstance()
{
	return FB_NEW(InputLayoutD3D11);
}

//----------------------------------------------------------------------------
InputLayoutD3D11::InputLayoutD3D11()
	: m_pInputLayout(0)
{

}

//----------------------------------------------------------------------------
InputLayoutD3D11::~InputLayoutD3D11()
{
	SAFE_RELEASE(m_pInputLayout);
}

//----------------------------------------------------------------------------
void InputLayoutD3D11::SetHardwareInputLayout(ID3D11InputLayout* pLayout)
{
	m_pInputLayout = pLayout;
}

//----------------------------------------------------------------------------
void InputLayoutD3D11::Bind()
{
	gFBEnv->pEngine->GetRenderer()->SetInputLayout(this);
}

void InputLayoutD3D11::SetDebugName(const char* name)
{
	if (m_pInputLayout){
		m_pInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
		m_pInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}