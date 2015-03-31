#include <UI/StdAfx.h>
#include <UI/ColorRampComp.h>
#include <UI/Button.h>
#include <UI/StaticText.h>

using namespace fastbird;

ColorRampComp::ColorRampComp()
{
	mUIObject = IUIObject::CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

ColorRampComp::~ColorRampComp()
{

}

void ColorRampComp::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;
	v.push_back(mUIObject);
	__super::GatherVisit(v);
}

bool ColorRampComp::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::COLOR_RAMP_VALUES:
	{
		SetColorRampValues(val);
		return true;
	}
	}

	return __super::SetProperty(prop, val);
}

bool ColorRampComp::GetProperty(UIProperty::Enum prop, char val[])
{
	switch (prop)
	{
	case UIProperty::COLOR_RAMP_VALUES:
	{
		GetColorRampValues(val, 6);
		return true;
	}
	}

	return __super::SetProperty(prop, val);
}

void ColorRampComp::SetColorRampValues(const char* values)
{
	if (!values || strlen(values) == 0)
	{
		return;
	}
	auto strs = Split(values, ", ");
	if (strs.size() < 2)
	{
		Log("void ColorRampComp::SetColorRampValues(const char* values) : 'values' should have at least two values.");
		assert(0);
		return;
	}
	
	std::vector<float> ratios;
	for (unsigned i = 0; i < strs.size(); i++)
	{
		ratios.push_back(StringConverter::parseReal(strs[i]));
	}
	SetColorRampValuesFloats(ratios);
}

void ColorRampComp::GetColorRampValues(char val[], int precision)
{
	std::vector<float> values;
	GetColorRampValuesFloats(values);

	std::string strValues;
	int i = 0;
	for (auto val : values)
	{
		strValues += StringConverter::toString(val, precision);
		if (i != values.size() - 1)
		{
			strValues += ",";
		}
	}
	strcpy(val, strValues.c_str());
}

void ColorRampComp::GetColorRampValuesFloats(std::vector<float>& values)
{
	values.clear();
	float prevXnpos = -1;
	for (unsigned i = 0; i < mBars.size(); i++)
	{
		auto& npos = mBars[i]->GetNPos();
		float xnpos = npos.x;
		if (prevXnpos != -1)
			xnpos -= prevXnpos;
		values.push_back(xnpos);
		prevXnpos = npos.x;
	}
	values.push_back(1.0f - prevXnpos);
}

void ColorRampComp::SetColorRampValuesFloats(const std::vector<float>& values)
{
	RemoveAllChild();
	mBars.clear();
	mTexts.clear();

	float prevXPos = -1;
	for (unsigned i = 0; i < values.size() - 1; i++)
	{
		float ratio = values[i];
		float xPos = ratio;
		if (prevXPos != -1)
		{
			xPos += prevXPos;
		}

		auto bar = AddChild(xPos, 0.5f, 0.05f, 1.0f, ComponentType::Button);
		mBars.push_back((Button*)bar);
		bar->SetSizeModificator(Vec2I(0, -10));
		bar->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
		bar->SetProperty(UIProperty::TEXTUREATLAS, "es/textures/ui.xml");
		bar->SetProperty(UIProperty::REGION, "BarComp");
		bar->SetProperty(UIProperty::HOVER_IMAGE, "BarCompHover");
		bar->SetProperty(UIProperty::DRAGABLE, "1, 0");

		float centerXPos;
		if (prevXPos == -1)
		{
			centerXPos = ratio * .5f;
		}
		else
		{
			centerXPos = prevXPos + ratio * .5f;
		}
		auto text = AddChild(centerXPos, 0.5f, 1, 1, ComponentType::StaticText);
		text->SetSize(Vec2I(50, 20));
		text->SetProperty(UIProperty::TEXT_ALIGN, "center");
		text->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
		char buf[255];
		sprintf_s(buf, "%d", (int)(ratio * 100));
		text->SetText(AnsiToWide(buf));
		mTexts.push_back((StaticText*)text);

		prevXPos = xPos;
	}
	float ratio = values.back();
	float centerXPos = prevXPos + ratio * .5f;
	auto text = AddChild(centerXPos, 0.5f, 1, 1, ComponentType::StaticText);
	text->SetSize(Vec2I(50, 20));
	text->SetProperty(UIProperty::TEXT_ALIGN, "center");
	text->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
	char buf[255];
	sprintf_s(buf, "%d", (int)(ratio * 100));
	text->SetText(AnsiToWide(buf));
	mTexts.push_back((StaticText*)text);
}

void ColorRampComp::OnChildHasDragged()
{
	if (mBars.empty())
		return;

	assert(mBars.size() == mTexts.size()-1);
	for (unsigned i = 0; i < mBars.size(); ++i)
	{
		auto npos = mBars[i]->GetNPos();
		float ratio = npos.x;
		if (i != 0)
		{
			ratio -= mBars[i - 1]->GetNPos().x;
		}
		char buf[255];
		sprintf_s(buf, "%d", (int)(ratio * 100));
		mTexts[i]->SetText(AnsiToWide(buf));
		if (i != 0)
		{
			mTexts[i]->SetNPos(Vec2(mBars[i - 1]->GetNPos().x + ratio*.5f, 0.5f));
		}
		else
		{
			mTexts[i]->SetNPos(Vec2(ratio*.5f, 0.5f));
		}
	}

	float xPos = mBars.back()->GetNPos().x;
	char buf[255];
	float ratio = 1.0f - xPos;
	sprintf_s(buf, "%d", (int)(ratio * 100));
	mTexts.back()->SetText(AnsiToWide(buf));
	mTexts.back()->SetNPos(Vec2(xPos + ratio*.5f, 0.5f));

	// issue the event
	OnEvent(IEventHandler::EVENT_COLORRAMP_DRAGGED);
}
