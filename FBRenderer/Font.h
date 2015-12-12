/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#include <vector>
#include <stack>
#include <map>
#include "RendererStructs.h"
#include "FBMathLib/Vec2I.h"
#include  "TextTags.h"

namespace fb{
	FB_DECLARE_SMART_PTR(TextureAtlas);
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(VertexBuffer);
	FB_DECLARE_SMART_PTR(Shader);
	FB_DECLARE_SMART_PTR(Material);
	struct SCharDescr;
	typedef DEFAULT_INPUTS::V_PCTB  FontVertex;
	FB_DECLARE_SMART_PTR(Font);
	class FB_DLL_RENDERER Font
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(Font);
		Font();
		~Font();

	public:
		enum EFontTextEncoding
		{
			NONE,
			UTF8,
			UTF16
		};

		enum FONT_ALIGN
		{
			FONT_ALIGN_LEFT = 0,
			FONT_ALIGN_CENTER,
			FONT_ALIGN_RIGHT,
			FONT_ALIGN_JUSTIFY
		};

		static FontPtr Create();				
		int Init(const char *fontFile);
		void Reload();
		const char* GetFilePath() const;
		void SetTextEncoding(EFontTextEncoding encoding);

		void PreRender(){}
		void Render(){}
		void PostRender(){}
		void Write(Real x, Real y, Real z, unsigned int color,
			const char *text, int count, FONT_ALIGN mode);

		int GetFontSize() const;
		void ScaleFontSizeTo(int desiredSize);
		void ScaleFontHeightTo(float desiredHeight);

		/// not scaled height
		Real GetOriginalHeight() const;
		/// scaled height
		Real GetHeight() const;
		Real GetBaseHeight() const;		
		Real GetTextWidth(const char *text, int count = -1, Real *minY = 0, Real *maxY = 0);
		std::wstring InsertLineFeed(const char *text, int count, unsigned wrapAt, Real* outWidth, unsigned* outLines);
		void PrepareRenderResources();
		void SetRenderStates(bool depthEnable = false, bool scissorEnable = false);

		Real GetBottomOffset();
		Real GetTopOffset();

		void SetRenderTargetSize(const Vec2I& rtSize);
		void RestoreRenderTargetSize();
		std::wstring StripTags(const wchar_t* text);
		void SetTextureAtlas(TextureAtlasPtr atlas);

	protected:
		friend class FontLoader;

		static const unsigned int MAX_BATCH;

		/// X will be modified.
		bool ApplyTag(const char* text, int start, int end, Real& x, Real y);
		TextTags::Enum GetTagType(const char* tagStart, int length, char* buf = 0) const;		
		void Flush(int page, const FontVertex* pVertices, unsigned int vertexCount);

		int AdjustForKerningPairs(int first, int second);
		SCharDescr *GetChar(int id);

		int GetTextLength(const char *text);
		int SkipTags(const char* text, TextTags::Enum* tag = 0, int* imgLen = 0);
		int GetTextChar(const char *text, int pos, int *nextPos = 0);
		int FindTextChar(const char *text, int start, int length, int ch);

		
	};
}