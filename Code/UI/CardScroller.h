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
		ICardData();
		virtual ~ICardData();

		virtual bool operator< (const ICardData& other) = 0;
	};

	//------------------------------------------------------------------------
	class CardItem : public Container
	{
	public:
		CardItem();
		virtual ~CardItem();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::CardScroller; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);

		virtual void OnStartUpdate(float elapsedTime);

		// will not have owner ship of the pointer.
		// before you delete the pointer, make sure this variable is null.
		virtual void SetCardData(ICardData* data) { mCardData = data; }
		
		bool operator< (const CardItem& other) const;


	private:
		ICardData* mCardData; // custom class;
		ImageBox* mBackground;
		ImageBox* mFrame;
		StaticText* mType_Name;
	};

	//------------------------------------------------------------------------
	class CardScroller : public Container
	{
	public:
		CardScroller();
		virtual ~CardScroller();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::CardScroller; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual void Sort();
		virtual void SetCardSize_Offset(const Vec2& x_ratio, int offset);
		virtual IWinBase* AddCard();
		virtual void ArrangeSlots();

	private:
		std::vector<Vec2> mCurrentPos;

		struct Slot
		{
			float mNYPos;
			bool mOccupied;

		};
		std::vector<Slot> mSlots;
		int mNextEmptySlot;
		float mWidth;
		float mHeight;
		float mRatio;
		float mNYOffset;
	};

}