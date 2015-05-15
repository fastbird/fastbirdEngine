#pragma once

namespace fastbird{
	class IKeyboard;
	class IMouse;
	class ITextManipulatorListener;
	class TextManipulator
	{
		int mCursorPos;
		int mHighlightStart;
		std::wstring* mText;
		std::vector<ITextManipulatorListener*> mListeners;

	private:
		
		void EndHighlighting() { mHighlightStart = -1; }
		void OnCursorPosChanged();
		void OnTextChanged();
		void Highlighting(bool shiftkey);
		void StartHighlighting();
		

	public:
		TextManipulator();
		~TextManipulator();
		virtual void AddListener(ITextManipulatorListener* l);
		virtual void RemoveListener(ITextManipulatorListener* l);
		virtual void SetText(std::wstring* text);
		virtual void OnInput(IMouse* mouse, IKeyboard* keyboard);
		virtual int GetCursorPos() const;
		bool IsHighlighting() const { return mHighlightStart != -1; }
		virtual int GetHighlightStart() const { return mHighlightStart; }
	};
}