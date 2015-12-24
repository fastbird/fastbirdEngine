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

#include "stdafx.h"
#include "ConsoleRenderer.h"
#include "Renderer.h"
#include "Font.h"
#include "FBConsole/Console.h"
using namespace fb;

static const int sFontSize = 20;
static const int sLineGap = 2;
static const int sCursorWidth = 10;
static const int sPromptStart = 2;
class ConsoleRenderer::Impl{
public:
	ConsoleRenderer* mSelf;
	int mHeight;
	int mLines;
	Vec2I mInputPosition;

	Impl(ConsoleRenderer* self)
		: mSelf(self)
		, mHeight()
		, mLines(15)
	{
		mHeight = (sFontSize + sLineGap) * mLines;
		mInputPosition = Vec2I(20, mHeight - sLineGap);
		if (Console::HasInstance()){
			Console::GetInstance().RegisterConsoleRenderer(mSelf);
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "You didn't create the Console before creating console renderer.");
		}
	}

	~Impl(){
		if (Console::HasInstance()){
			Console::GetInstance().RegisterConsoleRenderer(mSelf);
		}
	}

	void Render(){
		if (!Console::HasInstance())
			return;

		auto& console = Console::GetInstance();
		if (!console.IsOpen())
			return;
		
		auto& renderer = Renderer::GetInstance();
		FontPtr pFont = renderer.GetFont(sFontSize);
		const int lineHeight = (int)pFont->GetHeight();

		// Draw Background
		const auto& size = renderer.GetMainRenderTargetSize();
		renderer.DrawQuad(Vec2I(0, 0), Vec2I(size.x, mHeight),
			Color(0.15f, 0.15f, 0.15f, 0.5f));

		auto consoleInputPos = console.GetCursorPos();
		auto inputString = console.GetInputString();
		auto highlightStart = console.GetHighlightStart();
		// DrawHighlight
		if (highlightStart != -1)
		{
			Vec2I highlightStartPos = mInputPosition;
			highlightStartPos.x += (int)pFont->GetTextWidth((char*)inputString,
				highlightStart * 2);
			highlightStartPos.y -= lineHeight;
			Vec2I highlightEndPos = mInputPosition;
			highlightEndPos.x += (int)pFont->GetTextWidth((char*)inputString,
				consoleInputPos * 2);
			highlightEndPos.y -= lineHeight;

			if (highlightStart > consoleInputPos)
			{
				std::swap(highlightStartPos, highlightEndPos);
			}

			highlightEndPos.y += lineHeight;

			renderer.DrawQuad(highlightStartPos, highlightEndPos - highlightStartPos,
				Color::Gray);
		}

		// Draw Cursor
		Vec2I cursorPos = mInputPosition;
		cursorPos.x += (int)pFont->GetTextWidth((char*)inputString, consoleInputPos * 2);
		renderer.DrawQuad(cursorPos, Vec2I(sCursorWidth, 2), Color::Yellow);

		// Draw prompt
		auto prompt = console.GetPrompt();
		pFont->PrepareRenderResources();
		pFont->SetRenderStates();
		pFont->Write((float)sPromptStart, (float)mInputPosition.y, 0.f, Color::White.Get4Byte(),
			(char*)prompt, -1, Font::FONT_ALIGN_LEFT);

		// Draw Input String
		pFont->Write((float)mInputPosition.x, (float)mInputPosition.y, 0.f, Color::White.Get4Byte(),
			(char*)inputString, -1, Font::FONT_ALIGN_LEFT);



		// Draw Buffer
		auto bufferwRender = console.GetDisplayBuffer();
		int bufferDrawPosY = mInputPosition.y - lineHeight - sLineGap;
		auto it = bufferwRender.begin();
		auto itEnd = bufferwRender.end();
		for (; it < itEnd; it++)
		{
			unsigned numLineFeed = std::count(it->begin(), it->end(), L'\n');
			pFont->Write((float)mInputPosition.x, (float)bufferDrawPosY, 0.f,
				Color::Gray.Get4Byte(), (char*)it->c_str(), -1, Font::FONT_ALIGN_LEFT);
			bufferDrawPosY -= lineHeight + sLineGap;
		}		
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(ConsoleRenderer);
ConsoleRenderer::ConsoleRenderer()
	:mImpl(new Impl(this))
{

}

ConsoleRenderer::~ConsoleRenderer(){

}



void ConsoleRenderer::Render(){
	mImpl->Render();
}

int ConsoleRenderer::GetTextWidth(const wchar_t* text, int count, float *outMinY/* = 0*/, float *outMaxY/* = 0*/){
	auto font = Renderer::GetInstance().GetFont(sFontSize);
	if (!font)
		return 100;
	return (int)font->GetTextWidth((char*)text, count, outMinY, outMaxY);
}

std::wstring ConsoleRenderer::StripTags(const wchar_t* text){
	auto font = Renderer::GetInstance().GetFont(sFontSize);
	if (!font)
		return text;

	return font->StripTags(text);
}

int ConsoleRenderer::NumLinesRender(){
	return mImpl->mLines;
}

HWindow ConsoleRenderer::GetMainWindowHandle(){
	return Renderer::GetInstance().GetMainWindowHandle();
}