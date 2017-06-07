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
#include "TrailFacade.h"
#include "EngineFacade.h"
#include "FBRenderer/Material.h"
#include "FBSceneManager/Scene.h"
#include "FBSceneObjectFactory/TrailObject.h"
using namespace fb;
class TrailFacade::Impl{
public:
	TrailObjectPtr mTrail;
	Transformation mOffset;

	//---------------------------------------------------------------------------
	Impl()
	{
		mTrail = TrailObject::Create();
	}

	void SetOffset(const Transformation& offset){
		mOffset = offset;
	}

	void AddPoint(const Transformation& parentLocation){
		mTrail->AddPoint((parentLocation * mOffset).GetTranslation());
	}

	void SetWidth(float width){
		mTrail->SetWidth(width);
	}

	void SetWidthMultiply(float mul) {
		mTrail->SetWidthMultiply(mul);
	}

	void SetLengthMultiply(float mul) {
		mTrail->SetLengthMultiply(mul);
	}

	bool AttachToScene(){
		auto scene = EngineFacade::GetInstance().GetMainScene();
		if (!scene)
			return false;
		return scene->AttachObjectFB(mTrail);
	}

	bool DetachFromScene(){
		return mTrail->DetachFromScene();
	}

	MaterialPtr GetMaterial(){
		return mTrail->GetMaterial();
	}

};

//---------------------------------------------------------------------------
std::vector<TrailFacadeWeakPtr> sTrails;
//---------------------------------------------------------------------------
TrailFacadePtr TrailFacade::Create(){
	TrailFacadePtr p(new TrailFacade, [](TrailFacade* obj){ delete obj; });
	sTrails.push_back(p);
	return p;
}

TrailFacade::TrailFacade()
	:mImpl(new Impl)
{

}

TrailFacade::~TrailFacade(){

}

void TrailFacade::SetOffset(const Transformation& offset){
	mImpl->SetOffset(offset);
}

void TrailFacade::AddPoint(const Transformation& parentLocation){
	mImpl->AddPoint(parentLocation);
}

void TrailFacade::SetWidth(float width){
	mImpl->SetWidth(width);
}

void TrailFacade::SetWidthMultiply(float mul) {
	mImpl->SetWidthMultiply(mul);
}

void TrailFacade::SetLengthMultiply(float mul) {
	mImpl->SetLengthMultiply(mul);
}

bool TrailFacade::AttachToScene(){
	return mImpl->AttachToScene();
}

bool TrailFacade::DetachFromScene(){
	return mImpl->DetachFromScene();
}

MaterialPtr TrailFacade::GetMaterial(){
	return mImpl->GetMaterial();
}

void TrailFacade::SetDiffuseColor(const Color& color){
	auto mat = GetMaterial();
	if (mat){
		mat->SetDiffuseColor(color.GetVec4());
	}
}

void TrailFacade::Update(TIME_PRECISION dt){
	mImpl->mTrail->Update(dt);
}