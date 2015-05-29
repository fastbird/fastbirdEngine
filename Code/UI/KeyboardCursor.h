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

		void SetPos(const Vec2I& pos);
		void SetSize(const Vec2I& size);
		void SetHwndId(HWND_ID hwndId);
		void SetScissorRegion(const RECT& r);

	private:
		static KeyboardCursor* mInstance;
		IUIObject* mUIObject;
		bool mVisible;
		HWND_ID mHwndId;
	};
}