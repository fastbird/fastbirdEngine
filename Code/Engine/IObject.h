#pragma once
#ifndef _fastbird_IObject_header_included_
#define _fastbird_IObject_header_included_

#include <CommonLib/SmartPtr.h>
#include <CommonLib/Math/Vec3.h>
#include <Engine/RendererStructs.h>
#include <Engine/IEngineEventListener.h>

namespace fastbird
{
    class IScene;
	class IIndexBuffer;
	class IMaterial;
	class IInputLayout;
	class IVertexBuffer;
	class Animation;
	class IObject : public ReferenceCounter
	{
	public:
		enum ObjectFlag
		{
			OF_HIDE = 0x1,
			OF_QUERYABLE = 0x2,
			OF_IGNORE_ME = 0x4, // in scene
			OF_NO_DEPTH_PASS = 0x8,
			OF_HIGHLIGHT_DEDI = 0x10,
		};
		virtual ~IObject() {}
		
		// Defined in Object
        virtual void OnAttachedToScene(IScene* pScene) = 0;
        virtual void OnDetachedFromScene(IScene* pScene) = 0;
		virtual void SetMaterial(const char* name, int pass = 0) = 0;
		virtual void SetMaterial(IMaterial* pMat, int pass = 0) = 0;
		virtual IMaterial* GetMaterial(int pass = 0) const = 0;
		virtual void SetVertexBuffer(IVertexBuffer* pVertexBuffer) = 0;
		virtual void SetIndexBuffer(IIndexBuffer* pIndexBuffer) = 0;
		// override the input layout defined in material
		virtual void SetInputLayout(IInputLayout* i) = 0; 
		virtual void SetObjFlag(unsigned flag) = 0;
		virtual unsigned GetObjFlag() const = 0;
		virtual void ModifyObjFlag(unsigned flag, bool enable) = 0;
		virtual bool HasObjFlag(unsigned flag) = 0;
		virtual void SetShow(bool show) = 0;
		virtual bool GetShow() const = 0;
		virtual void SetGameType(int type) = 0;
		virtual int GetGameType() const = 0;
		virtual void SetGamePtr(void* ptr) = 0;
		virtual void* GetGamePtr() const = 0;
		virtual void SetRadius(float r) = 0;
		virtual void AttachToScene() = 0;
		virtual void DetachFromScene(bool includingRtt = false) = 0;
		virtual bool IsAttached(IScene* pScene=0) const = 0;
		virtual void SetName(const char* name) {}
		virtual const char* GetName() const { return ""; }
		virtual void RegisterEventListener(IObjectEventListener* listener) = 0;
		virtual void RemoveEventListener(IObjectEventListener* listener) = 0;

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
		virtual void SetEnableHighlight(bool highlight) {}
		const Animation* GetAnimation() const { return 0; }
	};
}

#endif //_fastbird_IObject_header_included_