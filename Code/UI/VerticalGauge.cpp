#include <UI/StdAfx.h>
#include <UI/VerticalGauge.h>
#include <Engine/IMaterial.h>

namespace fastbird
{

VerticalGauge::VerticalGauge()
:mPercentage(0), mMaximum(1.f)
, mBlink(false)
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->SetMaterial("es/Materials/UIVerticalGauge.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

	IMaterial* mat = mUIObject->GetMaterial();
	mGaugeColor = Color(1, 1, 1, 1);
	mBlinkColor = Color(1, 0, 0, 1);
	mat->SetMaterialParameters(1, mGaugeColor.GetVec4());
	mat->SetMaterialParameters(2, mBlinkColor.GetVec4());
	// x is lerp.
	mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
	Vec2 texcoords[4] = {
		Vec2(0.f, 1.f),
		Vec2(0.f, 0.f),
		Vec2(1.f, 1.f),
		Vec2(1.f, 0.f)
	};
	mUIObject->SetTexCoord(texcoords, 4);
}

VerticalGauge::~VerticalGauge()
{

}

void VerticalGauge::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);
}
void VerticalGauge::OnStartUpdate(float elapsedTime)
{
	if (mBlink)
	{
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(3, Vec4(sin(gEnv->pTimer->GetTime()*3.0f)*.5f+.5f, 0, 0, 0));
	}
}

void VerticalGauge::SetPercentage(float p)
{
	mPercentage = p;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(0, Vec4(p, mMaximum, 0, 0));
}

void VerticalGauge::SetMaximum(float m)
{
	mMaximum = m;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(0, Vec4(mPercentage, m, 0, 0));
}

void VerticalGauge::Blink(bool blink)
{
	mBlink = blink;
	if (!blink)
	{
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
	}
}

void VerticalGauge::SetGaugeColor(const Color& color)
{
	mGaugeColor = color;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(1, color.GetVec4());
}

void VerticalGauge::SetBlinkColor(const Color& color)
{
	mBlinkColor = color;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(2, color.GetVec4());
}

}