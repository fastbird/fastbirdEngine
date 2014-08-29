#pragma once

namespace fastbird
{
	class IUIObject;
	class KeyboardCursor
	{	
	public:
		KeyboardCursor();
		~KeyboardCursor();

		static void InitializeKeyboardCursor();
		static void FinalizeKeyboardCursor();
		static KeyboardCursor& GetKeyboardCursor();

		IUIObject* GetUIObject() const;

		void SetNPos(const Vec2& pos);
		void SetNSize(const Vec2& size);

	private:
		static KeyboardCursor* mInstance;
		IUIObject* mUIObject;
		bool mVisible;
	};
}