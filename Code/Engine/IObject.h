#pragma once
#ifndef _fastbird_IObject_header_included_
#define _fastbird_IObject_header_included_

#include <CommonLib/SmartPtr.h>
#include <CommonLib/Math/Vec3.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
    class IScene;
	class IIndexBuffer;
	class IMaterial;
	class IInputLayout;
	class IObject : public ReferenceCounter
	{
	public:
		enum ObjectFlag
		{
			OF_HIDE = 0x1,
			OF_QUERYABLE = 0x2,
		};
		virtual ~IObject() {}
		
		// Defined in Object
        virtual void OnAttachedToScene(IScene* pScene) = 0;
        virtual void OnDetachedFromScene(IScene* pScene) = 0;
		virtual void SetMaterial(const char* name) = 0;
		virtual void SetMaterial(IMaterial* pMat) = 0;
		virtual IMaterial* GetMaterial() const = 0;
		virtual void SetRasterizerState(const RASTERIZER_DESC& desc) = 0;
		virtual void SetBlendState(const BLEND_DESC& desc) = 0;
		virtual void SetDepthStencilState(const DEPTH_STENCIL_DESC& desc) = 0;
		virtual void SetVertexBuffer(IVertexBuffer* pVertexBuffer) = 0;
		virtual void SetIndexBuffer(IIndexBuffer* pIndexBuffer) = 0;
		// override the input layout defined in material
		virtual void SetInputLayout(IInputLayout* i) = 0; 
		virtual void SetObjFlag(unsigned flag) = 0;
		virtual unsigned GetObjFlag() const = 0;
		virtual void ModifyObjFlag(unsigned flag, bool enable) = 0;
		virtual void SetShow(bool show) = 0;
		virtual bool GetShow() const = 0;
		virtual void SetGameType(int type) = 0;
		virtual int GetGameType() const = 0;
		virtual void SetGamePtr(void* ptr) = 0;
		virtual void* GetGamePtr() const = 0;
		virtual void SetRadius(float r) = 0;
		virtual void AttachToScene() = 0;
		virtual void DetachFromScene() = 0;
		virtual bool IsAttached(IScene* pScene) const = 0;
		virtual void SetName(const char* name) {}
		virtual const char* GetName() const { return ""; }

		// defined in SpatialObject
		virtual void SetDistToCam(float dist){}
		virtual float GetDistToCam() const {return -1.f;} // only SpatialObject returns valid.
		virtual const Vec3& GetPos() const { return Vec3::ZERO; }
		virtual void SetPos(const Vec3& pos) {}

		// define these in render objects
		virtual IObject* Clone() const{ return 0;}
		virtual void PreRender() = 0;
		virtual void Render() = 0;		
		virtual void PostRender() = 0;
	};
}

#endif //_fastbird_IObject_header_included_