#pragma once
#ifndef _fastbird_Object_header_included_
#define _fastbird_Object_header_included_

#include <Engine/IObject.h>
#include <Engine/Renderer/IRenderState.h>
#include <Engine/IInputLayout.h>
#include <Engine/Renderer/RendererStructs.h>
#include <CommonLib/MaTH/BoundingVolume.h>

namespace fastbird
{
	class IScene;
	class IVertexBuffer;
	class IRasterizerState;
	class IBlendState;
	class IDepthStencilState;

	class Object : public IObject
	{
	public:
		Object();
		virtual ~Object();

		virtual void Clone(IObject* cloned) const;
		virtual void OnAttachedToScene(IScene* pScene);
        virtual void OnDetachedFromScene(IScene* pScene);
		virtual void SetMaterial(const char* name, int pass = 0);
		virtual void SetMaterial(IMaterial* pMat, int pass = 0);
		virtual IMaterial* GetMaterial(int pass = 0) const;
		virtual void SetRasterizerState(const RASTERIZER_DESC& desc);
		virtual void SetBlendState(const BLEND_DESC& desc);
		virtual void SetDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		virtual void SetVertexBuffer(IVertexBuffer* pVertexBuffer){}
		virtual void SetIndexBuffer(IIndexBuffer* pIndexBuffer){}
		// override the input layout defined in material
		virtual void SetInputLayout(IInputLayout* i){}
		virtual void SetObjFlag(unsigned flag);
		virtual unsigned GetObjFlag() const;
		virtual void ModifyObjFlag(unsigned flag, bool enable);
		virtual void SetShow(bool show);
		virtual bool GetShow() const;
		virtual void SetGameType(int type);
		virtual int GetGameType() const;
		virtual void SetGamePtr(void* ptr);
		virtual void* GetGamePtr() const;
		virtual void SetRadius(float r);
		virtual void AttachToScene();
		virtual void DetachFromScene();
		virtual bool IsAttached(IScene* pScene=0) const;
		virtual void ClearRenderStates();

		virtual void RegisterEventListener(IObjectEventListener* listener);
		virtual void RemoveEventListener(IObjectEventListener* listener);

		// own
		void BindRenderStates(unsigned stencilRef=0);
		BoundingVolume* GetBoundingVolume() const { return mBoundingVolume; }
		BoundingVolume* GetBoundingVolumeWorld() const { return mBoundingVolumeWorld; }

	private:
		friend class RendererD3D11;
		friend class UIObject; // for shared resources.
		IRasterizerState* GetRasterizerState();
		IBlendState* GetBlenderState();
		IDepthStencilState* GetDepthStencilState();

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
		SmartPtr<BoundingVolume> mBoundingVolume;
		SmartPtr<BoundingVolume> mBoundingVolumeWorld;

		unsigned mObjFlag;

	protected:
		SmartPtr<IRasterizerState> mRasterizerState;
		SmartPtr<IBlendState> mBlendState;
		SmartPtr<IDepthStencilState> mDepthStencilState;
		std::vector<IObjectEventListener*> mEventListener;
		int mGameType;
		void* mGamePtr;
		bool mDestructing;
	};
}

#endif //_fastbird_Object_header_included_