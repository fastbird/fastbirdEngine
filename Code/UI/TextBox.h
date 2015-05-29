#pragma once

#include <UI/Container.h>

namespace fastbird
{
	class IUIObject;
	class ImageBox;

	class TextBox : public Container
	{
	public:
		TextBox();
		virtual ~TextBox();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::TextBox; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
		virtual void SetText(const wchar_t* szText);
		void SetWNScollingOffset(const Vec2& offset);
		virtual unsigned GetTextBoxHeight() const;
		virtual void SetHwndId(HWND_ID hwndId);

	protected:
		const static float LEFT_GAP;
		virtual void OnPosChanged(bool anim);
		virtual void OnSizeChanged();
		virtual void CalcTextWidth(); // virtual for mutiline text

	private:
		int mCursorPos;
		bool mPasswd;
		bool mMatchHeight;
		ImageBox* mImage;
		std::string mStrBackImage;
		std::string mStrKeepRatio;
	};

}