#pragma once

#include <Engine/IFont.h>
#include <Engine/Renderer/RendererStructs.h>
#include <Engine/Renderer/Shaders/Constants.h>
#include <Engine/Foundation/Object.h>

namespace fastbird
{
	class ITexture;
	class IVertexBuffer;
	class IShader;
	class IInputLayout;

	struct SCharDescr
	{
		SCharDescr() : srcX(0), srcY(0), srcW(0), srcH(0), xOff(0), yOff(0), xAdv(0), page(0) {}

		short srcX;
		short srcY;
		short srcW;
		short srcH;
		short xOff;
		short yOff;
		short xAdv;
		short page;
		unsigned int chnl;

		std::vector<int> kerningPairs;
	};

	typedef DEFAULT_INPUTS::V_PCTB  FontVertex;

	//------------------------------------------------------------------------
	class Font : public IFont
	{
	public:
		Font();
		virtual ~Font();
		//--------------------------------------------------------------------
		// IFont
		//--------------------------------------------------------------------
		virtual int Init(const char *fontFile);
		virtual void SetTextEncoding(EFontTextEncoding encoding);

		virtual void PreRender(){}
		virtual void Render(){}		
		virtual void PostRender(){}
		virtual void Write(float x, float y, float z, unsigned int color, 
			const char *text, int count, FONT_ALIGN mode);

		virtual void SetHeight(float h);
		virtual float GetHeight() const;
		virtual void SetBackToOrigHeight();
		virtual float GetTextWidth(const char *text, int count = -1, float *minY = 0, float *maxY = 0);
		virtual void PrepareRenderResources();
		virtual void SetRenderStates(bool depthEnable = false, bool scissorEnable = false);
		virtual void SetDefaultConstants();

		float GetBottomOffset();
		float GetTopOffset();

	protected:
		friend class FontLoader;

		static const unsigned int MAX_BATCH;

		void InternalWrite(float x, float y, float z, const char *text, int count, float spacing = 0);
		void Flush(int page, const FontVertex* pVertices, unsigned int vertexCount);

		float AdjustForKerningPairs(int first, int second);
		SCharDescr *GetChar(int id);

		int GetTextLength(const char *text);
		int GetTextChar(const char *text, int pos, int *nextPos = 0);
		int FindTextChar(const char *text, int start, int length, int ch);

		short mFontHeight; // total height of the font
		short mBase;       // y of base line
		short mScaleW;
		short mScaleH;
		SCharDescr mDefChar;
		bool mHasOutline;

		float mScale;
		EFontTextEncoding mEncoding;

		unsigned int mColor;

		std::map<int, SCharDescr*> mChars;
		std::vector<SmartPtr<ITexture>> mPages;

		SmartPtr<IVertexBuffer> mVertexBuffer;
		unsigned int mVertexLocation;
		SmartPtr<IShader> mShader;
		SmartPtr<IInputLayout> mInputLayout;

		bool mInitialized;

		OBJECT_CONSTANTS mObjectConstants;

		SmartPtr<IRasterizerState> mScissorEnabledState;
		SmartPtr<IDepthStencilState> mDepthEnabledState;
	};
}