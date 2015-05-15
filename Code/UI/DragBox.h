#pragma once

namespace fastbird
{
	class DragBox
	{
		bool mStarted;
		Vec2I mStartPos;
		Vec2I mEndPos;
		IWinBase* mMouseOveredContainer;
	public:
		DragBox();
		~DragBox();
		void Start(const Vec2I& startPos);
		void PushCur(const Vec2I& pos);
		void End(const Vec2I& end);
		void Render();
		bool IsStarted() const { return mStarted; }

		Vec2I GetMin() const;
		Vec2I GetMax() const;

		void SetMouseOveredContainer(IWinBase* mouseOveredContainer){ mMouseOveredContainer = mouseOveredContainer; }
		IWinBase* GetMouseOveredContainer() const { return mMouseOveredContainer; }

	};
}