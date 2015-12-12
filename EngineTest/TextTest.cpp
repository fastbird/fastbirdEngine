#include "StdAfx.h"
#include "TextTest.h"
#include "FBEngineFacade/EngineFacade.h"
using namespace fb;

class TextTest::Impl{
public:

	void Update(){
		EngineFacade::GetInstance().QueueDrawText(Vec2I(0, 22), "'Earth defense fleets are incapacitated by invasion of the Empire fleets. All earthian fleets, abort your missions and return to Earth orbit, immediately.'", Color::White);
	}
};

FB_IMPLEMENT_STATIC_CREATE(TextTest);
TextTest::TextTest()
	: mImpl(new Impl){

}
TextTest::~TextTest(){

}

void TextTest::Update(){
	mImpl->Update();
}