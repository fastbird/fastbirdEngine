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
		virtual void SetTexCoord(Vec2 coord[], DWORD num, unsigned index=0);
		virtual void ClearTexCoord(unsigned index = 0);
		virtual void SetColors(DWORD colors[], DWORD num);

		virtual void SetUIPos(const Vec2I& pos);
		virtual const Vec2I& GetUIPos() const;

		virtual void SetUISize(const Vec2I& size);
		virtual const Vec2I& GetUISize() const;
		
		virtual void SetAlpha(float alpha);

		virtual void SetText(const wchar_t* s);
		virtual void SetTextOffset(const Vec2I& offset);
		virtual const Vec2I& GetTextOffset() const { return mTextOffset; }
		virtual void SetTextOffsetForCursorMovement(const Vec2I& offset);
		virtual Vec2I GetTextStartWPos() const;

		virtual void SetTextColor(const Color& c);
		virtual void SetTextSize(float size);
		virtual const RECT& GetRegion() const;
		virtual void SetDebugString(const char* string);
		virtual void SetNoDrawBackground(bool flag);
		virtual bool GetNoDrawBackground() const { return mNoDrawBackground; }
		virtual void SetUseScissor(bool use, const RECT& rect);
		virtual void SetSpecialOrder(int order) { mSpecialOrder = order; }
		virtual int GetSpecialOrder() const {
			return mSpecialOrder;
		}

		virtual void SetMultiline(bool multiline){ mMultiline = multiline; }
		virtual void SetDoNotDraw(bool doNotDraw);
		virtual void SetRenderTargetSize(const Vec2I& rtSize);
		virtual const Vec2I& GetRenderTargetSize() const;
		virtual bool HasTexCoord() const;

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
		//Vec2 mTextNPos;
		Vec2I mTextOffset;
		Vec2I mTextOffsetForCursor;
		Color mTextColor;
		float mTextSize;
		float mAlpha;
		RECT mRegion;
		//Vec2 mNSize; // normalized (0~1)
		//Vec2 mNPos;
		Vec2I mUISize;
		Vec2I mUIPos;
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
		bool mMultiline;

		bool mDoNotDraw;
		Vec2 mPivot;
		Vec2I mRenderTargetSize;
	};
}