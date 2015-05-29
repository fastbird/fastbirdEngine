#pragma once

#include <UI/Container.h>
namespace fastbird
{
	class IUIObject;
	class ImageBox;
	class StaticText;
	class Scroller;

	//------------------------------------------------------------------------
	class ICardData
	{
	public:
		virtual bool operator< (const ICardData& other) = 0;
	};

	//------------------------------------------------------------------------
	class CardItem : public Container
	{
	public:
		CardItem();
		virtual ~CardItem();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::CardItem; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);

		virtual void OnStartUpdate(float elapsedTime);

		virtual void SetCardData(ICardData* data) { mCardData = data; }
		
		bool operator< (const CardItem& other) const;

		unsigned GetCardId() const { return mCardId; }


	private:
		static unsigned NextCardId;
		unsigned mCardId;
		ICardData* mCardData; // custom class;
	};

	//------------------------------------------------------------------------
	class CardScroller : public Container
	{
	public:
		CardScroller();
		virtual ~CardScroller();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::CardScroller; }
		virtual void OnSizeChanged();
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
		virtual void Sort();
		virtual void SetCardSize_Offset(const Vec2& x_ratio, int offset);
		virtual void SetCardSize(const Vec2I& size);
		virtual void SetCardSizeNX(float nx);
		virtual void SetCardSizeX(int x);
		virtual void SetCardSizeY(int y);
		virtual void SetCardOffset(int offset);
		virtual IWinBase* AddCard();
		void AddCard(LuaObject& obj);
		virtual void DeleteCard(IWinBase* card);
		virtual void ArrangeSlots();

	private:

		std::vector<Vec2> mCurrentPos;

		struct Slot
		{
			float mNYPos;
			bool mOccupied;
			IWinBase* mCard;
			bool operator< (const Slot& other) const;
		};		
		std::vector<Slot> mSlots;
		Slot* GetNextCardPos(Vec2& outPos);

		int mNextEmptySlot;
		float mWidth;
		float mHeight;
		float mRatio;
		float mNYOffset;

		int mCardSizeX;
		int mCardSizeY;
		int mCardOffsetY;
	};

}