#pragma once

#include <UI/ComponentType.h>
#include <UI/Align.h>
#include <UI/UIProperty.h>
#include <UI/IUIAnimation.h>
#include <Engine/IUIObject.h>
#include <CommonLib/LuaObject.h>

namespace fastbird
{
	class IMouse;
	class IKeyboard;
	class IEventHandler;
	class IUIAnimation;

	class CLASS_DECLSPEC_UI IWinBase
	{
	public:
		IWinBase() {}
		virtual ~IWinBase() {}

		virtual void OnCreated() = 0;
		virtual ComponentType::Enum GetType() const = 0;
		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type) = 0;
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type) = 0;
		virtual IWinBase* AddChild(const fastbird::LuaObject& compTable) = 0;
		virtual IWinBase* AddChild(ComponentType::Enum type) = 0;
		virtual void RemoveChild(IWinBase* child, bool immediately=false) = 0;
		virtual void RemoveAllChild(bool immediately = false) = 0;
		virtual IWinBase* GetChild(const char* name, bool includeSubChildren = false) = 0;
		virtual IWinBase* GetChild(unsigned idx) = 0;
		virtual IWinBase* GetParent() = 0;
		virtual unsigned GetNumChildren() const = 0;
		virtual void RemoveAllEvents(bool includeChildren) = 0;

		virtual void SetSize(const fastbird::Vec2I& size) = 0;
		virtual void SetSizeX(int x) = 0;
		virtual void SetSizeY(int y) = 0;

		virtual void SetPos(const fastbird::Vec2I& pos) = 0;
		virtual void SetPosX(int x) = 0;
		virtual void SetPosY(int y) = 0;
		virtual void SetInitialOffset(Vec2I offset) = 0;

		virtual void SetNSize(const fastbird::Vec2& size) = 0; // normalized size (0.0~1.0)
		virtual void SetNSizeX(float x) = 0;
		virtual void SetNSizeY(float y) = 0;
		
		virtual void SetNPos(const fastbird::Vec2& pos) = 0; // normalized pos (0.0~1.0)
		virtual void SetNPosX(float x) = 0; // normalized pos (0.0~1.0)
		virtual void SetNPosY(float y) = 0; // normalized pos (0.0~1.0)


		virtual void SetWNSize(const fastbird::Vec2& size) = 0;
		virtual void SetWNPos(const fastbird::Vec2& wnPos) = 0;
		virtual const Vec2& GetNPos() const = 0;
		virtual const Vec2I& GetPos() const = 0;
		virtual const Vec2& GetWNPos() const = 0;
		virtual Vec2 GetFinalPos() const = 0;
		virtual const Vec2& GetWNSize() const = 0;
		virtual const Vec2& GetNSize() const = 0;
		virtual const Vec2I& GetSize() const = 0;
		virtual void SetName(const char* name) = 0;		
		virtual const char* GetName() const = 0;
		virtual void ClearName() = 0;
		virtual bool SetVisible(bool show) = 0;
		virtual bool SetVisibleChildren(bool show) = 0;
		virtual void SetVisibleInternal(bool visible) = 0;
		virtual void OnParentVisibleChanged(bool visible) = 0;
		virtual bool GetVisible() const = 0;
		virtual bool GetFocus(bool includeChildren = false) const = 0;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v) = 0;		
		virtual void GatherVisit(std::vector<IUIObject*>& v) = 0;
		virtual void SetSizeModificator(const Vec2I& sizemod) = 0;

		virtual void OnStartUpdate(float elapsedTime) = 0;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard) = 0;
		virtual IWinBase* FocusTest(IMouse* mouse) = 0;
		virtual void OnFocusLost() = 0;
		virtual void OnFocusGain() = 0;

		virtual void SetTextColor(const Color& c) = 0;
		virtual void SetText(const wchar_t* szText) = 0;
		virtual const wchar_t* GetText() const = 0;
		virtual void SetPasswd(bool passwd) = 0;

		virtual void SetNext(IWinBase* next) = 0;
		virtual void SetPrev(IWinBase* prev) = 0;
		virtual IWinBase* GetNext() const = 0;
		virtual IWinBase* GetPrev() const = 0;

		virtual IEventHandler* GetEventHandler() const = 0;

		virtual bool SetProperty(UIProperty::Enum prop, const char* val) = 0;
		virtual bool GetProperty(UIProperty::Enum prop, char val[]) = 0;
		virtual bool GetPropertyAsBool(UIProperty::Enum prop, bool defaultVal = false) = 0;
		virtual float GetPropertyAsFloat(UIProperty::Enum prop, float defaultVal = 0.f) = 0;
		virtual int GetPropertyAsInt(UIProperty::Enum prop, int defaultVal = 0) = 0;

		virtual void Scrolled() = 0;
		virtual void SetNPosOffset(const Vec2& offset) = 0;
		virtual const Vec2& GetNPosOffset() const = 0;
		virtual void SetAnimNPosOffset(const Vec2& offset) = 0;
		virtual void SetAnimScale(const Vec2& scale, const Vec2& povot) = 0;
		virtual void SetPivotToUIObject(const Vec2& pivot) = 0;
		virtual const RECT& GetRegion() const = 0;
		virtual Vec2 GetPivotWNPos() = 0;

		virtual float PixelToLocalNWidth(int pixel) const = 0;
		virtual float PixelToLocalNHeight(int pixel) const = 0;
		virtual Vec2 PixelToLocalNSize(const Vec2I& pixel) const = 0;
		virtual float PixelToLocalNPosX(int pixel) const = 0;
		virtual float PixelToLocalNPosY(int pixel) const = 0;
		virtual Vec2 PixelToLocalNPos(const Vec2I& pixel) const = 0;

		virtual IUIAnimation* GetOrCreateUIAnimation(const char* name) = 0;
		virtual IUIAnimation* GetUIAnimation(const char* name) = 0;
		virtual void SetUIAnimation(IUIAnimation* anim) = 0;
		virtual void ClearAnimationResult() = 0;

		virtual bool ParseXML(tinyxml2::XMLElement* pelem) = 0;
		virtual bool ParseLua(const fastbird::LuaObject& compTable) = 0;
		virtual float GetTextBottomGap() const = 0;

		// You usually controll the ui-layer by changing the order of Adding new UI. Later added ui draw first.
		// But if you use alpha blending, you need to use these enumeration by calling IWinBase::SetLayer()
		/*
		enum Layer
		{
			LAYER_UP_5,
			LAYER_UP_4,
			LAYER_UP_3,
			LAYER_UP_2,
			LAYER_UP_1,

			LAYER_DEFAULT,

			LAYER_DOWN_1,
			LAYER_DOWN_2,
			LAYER_DOWN_3,
			LAYER_DOWN_4,
			LAYER_DOWN_5,
		};
		*/
		virtual void SetAlphaBlending(bool set) = 0;
		/*virtual bool GetAlphaBlending() const = 0;*/

		virtual int GetTextWidth() const = 0;
		virtual float GetTextWidthLocal() const = 0;
		virtual float GetTextEndWLocal() const = 0;

		virtual void RefreshScissorRects() = 0;

		virtual void SetEnable(bool enable) = 0;
		virtual bool GetEnable(bool enable) const = 0;

		// usually don't need to use this functions
		// coordinates are decided by functions like SetNPos():for relative or SetPos() for absolute.
		virtual void SetUseAbsXPos(bool use) = 0;
		virtual void SetUseAbsYPos(bool use) = 0;
		virtual bool GetUseAbsXPos() const = 0;
		virtual bool GetUseAbsYPos() const = 0;
		virtual void SetUseAbsXSize(bool use) = 0;
		virtual void SetUseAbsYSize(bool use) = 0;
		virtual bool GetUseAbsXSize() const = 0;
		virtual bool GetUseAbsYSize() const = 0;

		virtual void SetContent(void* p) = 0;
		virtual void* GetContent() const = 0;
		virtual int GetSpecialOrder() const = 0;

		virtual bool GetInheritVisibleTrue() const =0 ;

		virtual void SetScriptPath(const char* path) = 0;
		virtual const char* GetScriptPath() const = 0;
		virtual void SetUIFilePath(const char* path) = 0;
		virtual const char* GetUIFilePath() const = 0;

		virtual void SetRender3D(bool render3D, const Vec2I& renderTargetSize) = 0;
		virtual bool GetRender3D() const = 0;
		virtual Vec2I GetRenderTargetSize() const = 0;
		virtual IWinBase* GetRootWnd() const = 0;

		virtual bool IsAlwaysOnTop() const = 0;
		virtual IUIObject* GetUIObj() const = 0;
		virtual bool GetCloseByEsc() const = 0;

		virtual void StartHighlight(float speed)=0;
		virtual void StopHighlight()=0;

		virtual bool GetNoBackground() const = 0;
		virtual const Color GetBackColor() = 0;
		virtual float GetAlpha() const = 0;
		virtual void OnAlphaChanged() = 0;
	protected:
		virtual void OnPosChanged() = 0;
		virtual void OnSizeChanged() = 0;
		virtual void OnEnableChanged() = 0;
		virtual void OnChildHasDragged()= 0;
	};
}