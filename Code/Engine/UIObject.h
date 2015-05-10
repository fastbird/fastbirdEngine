#pragma once
#include <Engine/IUIObject.h>
#include <../es/shaders/Constants.h>
namespace fastbird
{
	class UIObject : public IUIObject
	{
	public:
		UIObject();
		virtual ~UIObject();

		//-------------------------------------------------------------------------
		// IUIObject interfaces
		//-------------------------------------------------------------------------
		virtual void SetVertices(const Vec3* ndcPoints, int num,
			const DWORD* colors = 0, const Vec2* texcoords = 0);
		virtual void SetTexCoord(Vec2 coord[], DWORD num, unsigned index=0);
		virtual void SetColors(DWORD colors[], DWORD num);

		virtual void SetNSize(const Vec2& nsize); // in normalized space (0.0f ~ 1.0f)
		virtual const Vec2& GetNSize() const { return mNSize; }
		virtual void SetNPos(const Vec2& npos);// in normalized space 0.0f~1.0f
		virtual const Vec2& GetNPos() const { return mNPos; }
		virtual void SetNPosOffset(const Vec2& nposOffset);// in normalized space 0.0f~1.0f
		virtual void SetAnimNPosOffset(const Vec2& nposOffset); // in normalized space 0.0f~1.0f
		virtual const Vec2& GetAnimNPosOffset() const { return mAnimNOffset; }
		virtual void SetAnimScale(const Vec2& scale, const Vec2& pivot);
		virtual const Vec2& GetAnimScale() const{
			return mScale;
		}
		virtual void SetPivot(const Vec2& pivot) { mPivot = pivot; }
		virtual void SetAlpha(float alpha);
		virtual void SetText(const wchar_t* s);
		virtual void SetTextStartNPos(const Vec2& npos);
		virtual const Vec2& GetTextStarNPos() const { return mTextNPos; }

		virtual void SetTextColor(const Color& c);
		virtual void SetTextSize(float size);
		virtual const RECT& GetRegion() const;
		virtual void SetDebugString(const char* string);
		virtual void SetNoDrawBackground(bool flag);
		virtual bool GetNoDrawBackground() const { return mNoDrawBackground; }
		virtual void SetUseScissor(bool use, const RECT& rect);
		virtual void SetAlphaBlending(bool set);
		virtual bool GetAlphaBlending() const;
		virtual void SetSpecialOrder(int order) { mSpecialOrder = order; }
		virtual int GetSpecialOrder() const {
			return mSpecialOrder;
		}

		virtual void SetMultiline(bool multiline){ mMultiline = multiline; }
		virtual void SetDoNotDraw(bool doNotDraw);
		virtual void SetRenderTargetSize(const Vec2I& rtSize);
		virtual const Vec2I& GetRenderTargetSize() const;

		//-------------------------------------------------------------------------
		// IObject interfaces
		//-------------------------------------------------------------------------
		virtual void PreRender();
		virtual void Render();		
		virtual void PostRender();

		virtual void SetMaterial(const char* name, int pass = 0);
		virtual IMaterial* GetMaterial(int pass =0) const { return mMaterial; }

		//-------------------------------------------------------------------------
		// Own
		//-------------------------------------------------------------------------
		void UpdateRegion();
		static void ClearSharedRS();

	private:
		void PrepareVBs();

	private:
		static SmartPtr<IRasterizerState> mRasterizerStateShared;
		SmartPtr<IMaterial> mMaterial;
		SmartPtr<IVertexBuffer> mVertexBuffer;
		OBJECT_CONSTANTS mObjectConstants; // for ndc space x, y;
		
		std::wstring mText;
		Vec2 mNDCPos; // ndc pos
		Vec2 mNDCOffset;
		Vec2 mNOffset;
		Vec2 mAnimNDCOffset;
		Vec2 mAnimNOffset;
		Vec2 mScale;
		Vec2 mTextNPos;
		Color mTextColor;
		float mTextSize;
		float mAlpha;
		RECT mRegion;
		Vec2 mNSize; // normalized (0~1)
		Vec2 mNPos;
		std::string mDebugString;
		bool mNoDrawBackground;
		std::vector<Vec3> mPositions;
		std::vector<DWORD> mColors;
		std::vector<Vec2> mTexcoords[2];
		SmartPtr<IVertexBuffer> mVBColor;
		SmartPtr<IVertexBuffer> mVBTexCoords[2];
		RECT mScissorRect;
		int mSpecialOrder;
		bool mDirty;
		bool mScissor;
		bool mOut;
		bool mAlphaBlending;
		bool mMultiline;

		bool mDoNotDraw;
		Vec2 mPivot;
		Vec2I mRenderTargetSize;
	};
}