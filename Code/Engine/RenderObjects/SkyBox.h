#pragma once
#include <Engine/Foundation/Object.h>
#include <Engine/ISkyBox.h>

namespace fastbird
{
	class SkyBox : public ISkyBox
	{
	public:
		SkyBox();
		virtual ~SkyBox();

		//------------------------------------------------------------------------
		// IObject
		//------------------------------------------------------------------------
		virtual void PreRender(){}
		virtual void Render();		
		virtual void PostRender(){}
		virtual IMaterial* GetMaterial(int pass = 0) const { return mMaterial; }

		//--------------------------------------------------------------------
		// ISkyBox Interfaces
		//--------------------------------------------------------------------
		virtual void Init();



	private:
		SmartPtr<IVertexBuffer> mVertexBuffer;
		SmartPtr<IMaterial> mMaterial;

	};
};