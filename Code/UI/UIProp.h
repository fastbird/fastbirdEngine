#pragma once
#include <UI/UIPropTypes.h>
#include <UI/UIProperty.h>

namespace fastbird
{
	// NOT USING
	template <class T>
	class UIProp
	{
	public:
		UIProp(UIProperty::Enum propname, UIPropTypes::Enum type, T defaultValue)
			: mPropName(propname)
			, mType(type)
			, mValue(defaultValue)
			, mDefaultValue(defaultValue)
		{
		}
		const T& GetValue() const { return mValue; }
		const char* GetStrValue() const { return mStrValue.c_str(); }
		
		void SetValue(const T& value){
			mVlaue = value;
		}
		void SetStrValue(const char* strValue)	{
			if (!strValue)
				return;

			mStrValue = strValue;
			switch (mType)
			{
			case UIPropTypes::Bool:
			{
				mValue = StringConverter::parseBool(strValue);
			}
			case UIPropTypes::Int:
			{
				mValue = StringConverter::parseInt(strValue);
			}
			case UIPropTypes::Color:
			{
				mValue = StringConverter::parseColor(strValue);
			}
			case UIPropTypes::Vec2I:
			{
				mValue = StringConverter::parseVec2I(strValue);
			}
			case UIPropTypes::Float:
			{
				mValue = StringConverter::parseReal(strValue);
			}
			case UIPropTypes::String:
			{
				mValue = mStrValue;
			}
			default:
				assert(0);
			}
		}

		bool IsDefault() const {
			return mValue == mDefaultValue;
		}



	private:
		UIProperty::Enum mPropName;
		UIPropTypes::Enum mType;
		T mValue;
		T mDefaultValue;

		std::string mStrValue;

	};
}