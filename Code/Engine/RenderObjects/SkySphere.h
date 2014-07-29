#pragma once
#include <Engine/ISkySphere.h>
namespace fastbird
{
	class IVertexBuffer;
	class IIndexBuffer;
	class SkySphere : public ISkySphere
	{
	public:		
		SkySphere();
		virtual ~SkySphere();

		// interfaces
		virtual void SetMaterial(const char* name);
		virtual void SetMaterial(IMaterial* pMat);
		virtual IMaterial* GetMaterial() const;

		virtual void PreRender();
		virtual void Render();		
		virtual void PostRender();

		// own
		void GenerateSphereMesh();

	private:
		SmartPtr<IMaterial> mMaterial;
		SmartPtr<IVertexBuffer> mVB;
		SmartPtr<IIndexBuffer> mIB;
		
	};
}