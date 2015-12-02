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

#include "StdAfx.h"
#include "DragBox.h"

namespace fb
{
DragBox::DragBox()
	: mStarted(false)
	, mStartPos(0, 0)
	, mEndPos(0, 0)
{

}

DragBox::~DragBox(){

}
void DragBox::Start(const Vec2I& startPos){
	mStarted = true;
	mStartPos = startPos;
}
void DragBox::PushCur(const Vec2I& pos){
	mEndPos = pos;
}
void DragBox::End(const Vec2I& end){
	mStarted = false;
	mEndPos = end;
}
void DragBox::Render(){
	if (mStarted)
	{
		Vec2I start = GetMinComp(mStartPos, mEndPos);
		Vec2I end = GetMaxComp(mStartPos, mEndPos);
		
		Renderer::GetInstance().QueueDrawQuad(start, end - start, Color(1.f, 1.f, 0.f, 0.3f));
	}
}


Vec2I DragBox::GetMin() const
{
	return GetMinComp(mStartPos, mEndPos);
}
Vec2I DragBox::GetMax() const
{
	return GetMaxComp(mStartPos, mEndPos);
}

}