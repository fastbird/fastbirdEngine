#include <Engine/StdAfx.h>
#include <Engine/Renderer/D3D11/InputLayoutD3D11.h>
#include <Engine/IEngine.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>

using namespace fastbird;

//----------------------------------------------------------------------------
InputLayoutD3D11* InputLayoutD3D11::CreateInstance()
{
	return new InputLayoutD3D11();
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