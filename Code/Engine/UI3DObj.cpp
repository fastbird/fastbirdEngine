#include <Engine/StdAfx.h>
#include <Engine/UI3DObj.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <Engine/IFont.h>
#include <Engine/IConsole.h>
#include <Engine/ICamera.h>
#include <Engine/EngineCommand.h>
#include <UI/ComponentType.h>


namespace fastbird
{
	const float UI3DObj::NotDefined = 12345.12345f;
	//---------------------------------------------------------------------------
	UI3DObj::UI3DObj()
		: mLastPos(NotDefined, NotDefined, NotDefined)
		, mLastCameraPos(NotDefined, NotDefined, NotDefined)
	{
		mObjectConstants.gWorld.MakeIdentity();
		mObjectConstants.gWorldViewProj.MakeIdentity();
		SetMaterial("es/materials/UI3D.material");
	}
	UI3DObj::~UI3DObj()
	{
	}
	

	//---------------------------------------------------------------------------
	void UI3DObj::SetMaterial(const char* name, int pass /*= RENDER_PASS::PASS_NORMAL*/)
	{
		mMaterial = fastbird::IMaterial::CreateMaterial(name);
		assert(mMaterial);
	}

	//----------------------------------------------------------------------------
	void UI3DObj::PreRender()
	{
		if (mObjFlag & IObject::OF_HIDE )
			return;

		if (mLastPreRendered == gFBEnv->mFrameCounter)
			return;
		mLastPreRendered = gFBEnv->mFrameCounter;

		mTransformation.GetHomogeneous(mObjectConstants.gWorld);

	}

	void UI3DObj::Render()
	{
		D3DEventMarker mark("UI3DObj Render()");

		if (mObjFlag & IObject::OF_HIDE || !mMaterial || gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL
			|| !gFBEnv->pConsole->GetEngineCommand()->r_UI)
			return;
		auto pRenderer = gFBEnv->pRenderer;
		mObjectConstants.gWorldView = pRenderer->GetCamera()->GetViewMat() * mObjectConstants.gWorld;
		mObjectConstants.gWorldViewProj = pRenderer->GetCamera()->GetViewProjMat() * mObjectConstants.gWorld;
		pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);

		pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		mMaterial->Bind(true);
		mVertexBuffer->Bind();
		pRenderer->Draw(4, 0);
	}

	void UI3DObj::PostRender()
	{
		if (mObjFlag & IObject::OF_HIDE)
			return;
	}

	void UI3DObj::SetPosSize(const Vec3& pos, const Vec2& sizeInWorld)
	{
		if (mPos != pos)
		{
			int a = 0;
			a++;
		}
		mPos = pos;
		mSizeInWorld = sizeInWorld;
		if (!mVertexBuffer)
		{
			mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(0, 
				sizeof(DEFAULT_INPUTS::V_PT), 4, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		}
		auto mapData = mVertexBuffer->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
		if (mapData.pData)
		{
			DEFAULT_INPUTS::V_PT data[4] = {
				{ Vec3(0, 0, -sizeInWorld.y), Vec2(0, 1) },
				{ Vec3(0, 0, sizeInWorld.y), Vec2(0, 0) },
				{ Vec3(sizeInWorld.x, 0, -sizeInWorld.y), Vec2(1, 1) },
				{ Vec3(sizeInWorld.x, 0, +sizeInWorld.y), Vec2(1, 0) },
			};
			/*for (int i = 0; i < 4; i++)
			{
				data[i].p = gFBEnv->pRenderer->GetCamera()->GetRot() * data[i].p;
			}*/
			auto size = sizeof(data);
			memcpy(mapData.pData, data, size);
			mVertexBuffer->Unmap();
		}

		float dt = gpTimer->GetDeltaTimeNotPausable();
		if (mLastPos.x == NotDefined)
		{
			mLastPos = pos;
		}
		else
		{
			mLastPos = Lerp(mLastPos, pos, std::min(1.0f, 3.0f * dt));
		}

		if (mLastCameraPos.x == NotDefined)
		{
			mLastCameraPos = gFBEnv->pRenderer->GetCamera()->GetPos();
		}
		else
		{
			mLastCameraPos = Lerp(mLastCameraPos, gFBEnv->pRenderer->GetCamera()->GetPos(), std::min(1.0f, 3.0f * dt));
		}

		auto toUI = (mLastPos - mLastCameraPos).NormalizeCopy();
		auto right = toUI.Cross(gFBEnv->pRenderer->GetCamera()->GetUp()).NormalizeCopy();
		auto up = right.Cross(toUI);
		
		Transformation t;
		t.SetRotation(Mat33(right, toUI, up));
		t.SetTranslation(pos);
		SetTransform(t);

		float	radius = sizeInWorld.Length();
		mBoundingVolume->SetCenter(Vec3(0, 0, 0));
		mBoundingVolume->SetRadius(radius);
		mBoundingVolumeWorld->SetCenter(pos);
		mBoundingVolumeWorld->SetRadius(radius);
	}

	void UI3DObj::Reset3DUI()
	{
		mLastPos.x = NotDefined;
		mLastCameraPos.x = NotDefined;
	}

	void UI3DObj::SetTexture(ITexture* texture)
	{
		assert(mMaterial);
		mMaterial->SetTexture(texture, BINDING_SHADER_PS, 0);
	}
}