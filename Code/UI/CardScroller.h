#pragma once

#include <UI/Container.h>
namespace fastbird
{
	class IUIObject;
	class ImageBox;
	class StaticText;
	class Scroller;
	class CardData;
	class Wnd;

	//------------------------------------------------------------------------
	class CardScroller : public Container
	{
		CardData* mCardData;
		std::vector<Wnd*> mItems;
		std::vector<Wnd*> mRecycleBin;
		struct Props{
			Props(const char* c, UIProperty::Enum p, const char* v)
				: compName(c)
				, prop(p)
				, val(v)
			{
			}
			std::string compName;
			UIProperty::Enum prop;
			std::string val;
		};
		// <key, Props>
		VectorMap<unsigned, std::vector<Props>> mItemProps;
		// < index, key >
		VectorMap<unsigned, unsigned> mKeys; // find keys with index
		float mWidth;
		float mHeight;
		float mRatio;
		float mNYOffset;

		int mCardSizeX;
		int mCardSizeY;
		int mCardOffsetY;

		unsigned mStartIndex;
		unsigned mEndIndex;

	public:
		CardScroller();
		virtual ~CardScroller();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::CardScroller; }
		virtual void OnSizeChanged();
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
		virtual void SetCardSize_Offset(const Vec2& x_ratio, int offset);
		virtual void SetCardSize(const Vec2I& size);
		virtual void SetCardSizeNX(float nx);
		virtual void SetCardSizeX(int x);
		virtual void SetCardSizeY(int y);
		virtual void SetCardOffset(int offset);
		
		
		virtual unsigned AddCard(unsigned key, LuaObject& data);
		virtual void DeleteCard(unsigned key);
		virtual void DeleteCardWithIndex(unsigned index);
		virtual void DeleteAllCard();
		virtual void SetTexture(unsigned key, const char* comp, ITexture* texture);
		
		void SetItemProperty(unsigned key, const char* comp, const char* prop, const char* val);

		void VisualizeData(unsigned index);
		void Scrolled();
		void MoveToRecycle(unsigned index);
		Wnd* CreateNewCard(unsigned index);
		bool IsExisting(unsigned key);
		void RefreshTextures(unsigned index);
		void RefreshProps(unsigned index);
	};

}