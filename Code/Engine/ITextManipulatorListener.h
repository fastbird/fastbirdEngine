#pragma once

namespace fastbird{
	class TextManipulator;
	class ITextManipulatorListener
	{
	public:
		virtual void OnCursorPosChanged(TextManipulator* mani) {}
		virtual void OnTextChanged(TextManipulator* mani) {}
	};
}