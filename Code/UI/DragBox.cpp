#include <UI/StdAfx.h>
#include <UI/DragBox.h>

namespace fastbird
{
DragBox::DragBox()
	: mStarted(false)
	, mStartPos(0, 0)
	, mEndPos(0, 0)
	, mMouseOveredContainer(0)
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
		
		gFBEnv->pRenderer->DrawQuad(start, end - start, Color(1.f, 1.f, 0.f, 0.3f));
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