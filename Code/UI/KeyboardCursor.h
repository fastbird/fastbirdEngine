#pragma once

namespace fastbird
{
	class IUIObject;
	class KeyboardCursor
	{	
		KeyboardCursor();
		~KeyboardCursor();

	public:
		static void InitializeKeyboardCursor();
		static void FinalizeKeyboardCursor();
		static KeyboardCursor& GetKeyboardCursor();

		IUIObject* GetUIObject() const;

		void SetNPos(const Vec2& pos);
		void SetNSize(const Vec2& size);

	private:
		static KeyboardCursor* mInstance;
		SmartPtr<IUIObject> mUIObject;
		bool mVisible;
	};
}