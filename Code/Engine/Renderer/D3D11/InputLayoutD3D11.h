#pragma once

#include <Engine/Renderer/Inputlayout.h>
#include <D3D11.h>
namespace fastbird
{
	class InputLayoutD3D11 : public InputLayout
	{
	public:
		static InputLayoutD3D11* CreateInstance();

		//--------------------------------------------------------------------
		// IInputLayout Interfaces
		//--------------------------------------------------------------------
		virtual void Bind();

		//--------------------------------------------------------------------
		// Own Interfaces
		//--------------------------------------------------------------------
		void SetHardwareInputLayout(ID3D11InputLayout* pLayout);
		ID3D11InputLayout* GetHardwareInputLayout() const { return m_pInputLayout; }

	protected:
		InputLayoutD3D11();
		virtual ~InputLayoutD3D11();

	private:
		ID3D11InputLayout* m_pInputLayout;
	};
}