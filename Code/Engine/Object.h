#pragma once
#ifndef _fastbird_Object_header_included_
#define _fastbird_Object_header_included_

#include <Engine/IObject.h>
#include <Engine/IRenderState.h>
#include <Engine/IInputLayout.h>
#include <Engine/RendererStructs.h>
#include <CommonLib/MaTH/BoundingVolume.h>

namespace fastbird
{
	class IScene;
	class IVertexBuffer;

	class Object : public IObject
	{
	protected:
		unsigned mLastPreRendered;

	public:
		Object();
		virtual ~Object();

	protected:
		virtual void FinishSmartPtr();

	public:
		virtual void Clone(IObject* cloned) const;
		virtual void OnAttachedToScene(IScene* pScene);
        virtual void OnDetachedFromScene(IScene* pScene);
		virtual void SetMaterial(const char* name, int pass = 0);
		virtual void SetMaterial(IMaterial* pMat, int pass = 0);
		virtual IMaterial* GetMaterial(int pass = 0) const;
		virtual void SetVertexBuffer(IVertexBuffer* pVertexBuffer){}
		virtual void SetIndexBuffer(IIndexBuffer* pIndexBuffer){}
		// override the input layout defined in material
		virtual void SetInputLayout(IInputLayout* i){}
		virtual void SetObjFlag(unsigned flag);
		virtual unsigned GetObjFlag() const;
		virtual void ModifyObjFlag(unsigned flag, bool enable);
		virtual bool HasObjFlag(unsigned flag);
		virtual void SetShow(bool show);
		virtual bool GetShow() const;
		virtual void SetGameType(int type);
		virtual int GetGameType() const;
		virtual void SetGamePtr(void* ptr);
		virtual void* GetGamePtr() const;
		virtual void SetRadius(float r);
		virtual void AttachToScene();
		virtual void DetachFromScene(bool includingRtt = false);
		virtual bool IsAttached(IScene* pScene=0) const;

		virtual void RegisterEventListener(IObjectEventListener* listener);
		virtual void RemoveEventListener(IObjectEventListener* listener);

		// need to move to SpatialObject
		BoundingVolume* GetBoundingVolume() const { return mBoundingVolume; }
		BoundingVolume* GetBoundingVolumeWorld() const { return mBoundingVolumeWorld; }

	private:
		friend class RendererD3D11;
		friend class UIObject; // for shared resources.

	protected:
		std::vector<IScene*> mScenes;

		//--------------------------------------------------------------------
		// Declare following classes in the inherited class when neccessary.
		/*--------------------------------------------------------------------
		SmartPtr<IVertexBuffer> mVertexBuffer;
		SmartPtr<IIndexBuffer> mIndexBuffer;
		SmartPtr<IMaterial> mMaterial;
		SmartPtr<IInputLayout> mInputLayoutOverride;
		*/

		//--------------------------------------------------------------------
		// The following classes are already instanciated in Object constructor
		//--------------------------------------------------------------------
		// need to move to SpatialObject
		SmartPtr<BoundingVolume> mBoundingVolume;
		SmartPtr<BoundingVolume> mBoundingVolumeWorld;

		unsigned mObjFlag;

	protected:
		std::vector<IObjectEventListener*> mEventListener;
		int mGameType;
		void* mGamePtr;
	};
}

#endif //_fastbird_Object_header_included_