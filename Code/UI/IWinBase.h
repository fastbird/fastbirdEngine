#pragma once

#include <UI/ComponentType.h>
#include <UI/Align.h>
#include <UI/UIProperty.h>
#include <UI/IUIAnimation.h>
#include <UI/RegionTestParam.h>
#include <UI/IEventHandler.h>
#include <Engine/IUIObject.h>
#include <CommonLib/LuaObject.h>

namespace fastbird
{
	class IMouse;
	class IKeyboard;
	class IEventHandler;
	class IUIAnimation;

	class IWinBase
	{
	public:
		IWinBase() {}
		virtual ~IWinBase() {}

		virtual void SetHwndId(HWND_ID hwndId) = 0;
		virtual HWND_ID GetHwndId() const = 0;
		virtual void OnCreated() = 0;
		virtual ComponentType::Enum GetType() const = 0;
		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type) = 0;
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type) = 0;
		virtual IWinBase* AddChild(const fastbird::LuaObject& compTable) = 0;
		virtual IWinBase* AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type) = 0;
		virtual IWinBase* AddChild(ComponentType::Enum type) = 0;
		virtual void RemoveChild(IWinBase* child, bool immediately=false) = 0;
		virtual void RemoveChildNotDelete(IWinBase* child) = 0;
		virtual void RemoveAllChild(bool immediately = false) = 0;
		virtual IWinBase* GetChild(const std::string& name, bool includeSubChildren = false) = 0;
		virtual IWinBase* GetChild(unsigned idx) = 0;
		virtual IWinBase* GetParent() = 0;
		virtual unsigned GetNumChildren(bool excludeRunTimeChild = false) const = 0;
		virtual void RemoveAllEvents(bool includeChildren) = 0;

		virtual void ChangeSize(const Vec2I& size)=0;
		virtual void ChangeSizeX(int sizeX) = 0;
		virtual void ChangeSizeY(int sizeY) = 0;
		virtual void SetSize(const Vec2I& size) = 0;		
		virtual void SetSizeX(int x) = 0;		
		virtual void SetSizeY(int y) = 0;

		virtual void ChangeNSize(const Vec2& nsize) = 0;
		virtual void ChangeNSizeX(float x) = 0;
		virtual void ChangeNSizeY(float y) = 0;
		virtual void SetNSize(const Vec2& size) = 0;
		virtual void SetNSizeX(float x) = 0;
		virtual void SetNSizeY(float y) = 0;
		virtual void SetWNSize(const fastbird::Vec2& size) = 0;
		virtual void OnParentSizeChanged() = 0;

		virtual void ChangePos(const Vec2I& pos) = 0; 
		virtual void ChangePosX(int posx) = 0;
		virtual void ChangePosY(int posy) = 0;
		virtual void ChangeWPos(const Vec2I& wpos) = 0;
		virtual void ChangeNPos(const Vec2& npos) = 0;
		virtual void ChangeNPosX(float xpos) = 0;
		virtual void ChangeNPosY(float ypos) = 0;
		virtual void SetPos(const Vec2I& pos) = 0;
		virtual void SetPosX(int x) = 0;
		virtual void SetPosY(int y) = 0;
		virtual void SetNPos(const Vec2& pos) = 0; // normalized pos (0.0~1.0)
		virtual void SetNPosX(float x) = 0; // normalized pos (0.0~1.0)
		virtual void SetNPosY(float y) = 0; // normalized pos (0.0~1.0)
		virtual void SetWNPos(const Vec2& wnPos) = 0;
		virtual void OnParentPosChanged() = 0;

		virtual void SetInitialOffset(Vec2I offset) = 0;
		virtual void Move(Vec2I amount) = 0;
		virtual void NotifyPosChange() = 0;	
		
		virtual const Vec2& GetNPos() const = 0;
		virtual const Vec2I& GetPos() const = 0;
		virtual const Vec2I GetAlignedPos() const = 0;
		virtual const Vec2& GetWNPos() const = 0;
		virtual const Vec2I& GetWPos() const = 0;
		virtual const Vec2I& GetFinalPos() const = 0;
		virtual const Vec2I& GetFinalSize() const = 0;
		virtual void SetWPos(const Vec2I& wpos) = 0;
		//virtual const Vec2& GetWNSize() const = 0;
		virtual const Vec2& GetNSize() const = 0;
		virtual const Vec2I& GetSize() const = 0;
		virtual void SetName(const char* name) = 0;		
		virtual const char* GetName() const = 0;
		virtual void ClearName() = 0;
		virtual bool IsIn(const Vec2I& pt, bool ignoreScissor, Vec2I* expand = 0) const = 0;

		virtual bool IsPtOnLeft(const Vec2I& pt, int area) const = 0;
		virtual bool IsPtOnRight(const Vec2I& pt, int area) const = 0;
		virtual bool IsPtOnTop(const Vec2I& pt, int area) const = 0;
		virtual bool IsPtOnBottom(const Vec2I& pt, int area) const = 0;

		virtual bool SetVisible(bool show) = 0;
		virtual bool SetVisibleChildren(bool show) = 0;
		virtual void SetVisibleInternal(bool visible) = 0;
		virtual void OnParentVisibleChanged(bool visible) = 0;
		virtual bool GetVisible() const = 0;
		virtual bool GetFocus(bool includeChildren = false) const = 0;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v) = 0;		
		virtual void GatherVisit(std::vector<IUIObject*>& v) = 0;
		virtual void ModifySize(const Vec2I& sizemod) = 0;

		virtual void OnStartUpdate(float elapsedTime) = 0;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard) = 0;
		virtual void OnFocusLost() = 0;
		virtual void OnFocusGain() = 0;

		virtual void SetTextColor(const Color& c) = 0;
		virtual void SetText(const wchar_t* szText) = 0;
		virtual const wchar_t* GetText() const = 0;
		virtual void SetPasswd(bool passwd) = 0;
		virtual int GetTextWidth() const = 0;
		virtual int GetTextEndPosLocal() const = 0;

		virtual void SetNext(IWinBase* next) = 0;
		virtual void SetPrev(IWinBase* prev) = 0;
		virtual IWinBase* GetNext() const = 0;
		virtual IWinBase* GetPrev() const = 0;

		virtual IEventHandler* GetEventHandler() const = 0;

		virtual bool SetProperty(UIProperty::Enum prop, const char* val) = 0;
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly) = 0;
		virtual bool GetPropertyAsBool(UIProperty::Enum prop, bool defaultVal = false) = 0;
		virtual float GetPropertyAsFloat(UIProperty::Enum prop, float defaultVal = 0.f) = 0;
		virtual int GetPropertyAsInt(UIProperty::Enum prop, int defaultVal = 0) = 0;

		virtual void Scrolled() = 0;
		virtual void SetWNScollingOffset(const Vec2& offset) = 0;
		virtual const Vec2& GetWNScrollingOffset() const = 0;
		virtual void SetAnimScale(const Vec2& scale) = 0;
		//virtual void SetPivotToUIObject(const Vec2& pivot) = 0;
		virtual const RECT& GetRegion() const = 0;
		//virtual Vec2 GetPivotWNPos() = 0;

		virtual IUIAnimation* GetOrCreateUIAnimation(const char* name) = 0;
		virtual IUIAnimation* GetUIAnimation(const char* name) = 0;
		virtual void SetUIAnimation(IUIAnimation* anim) = 0;
		virtual void ClearAnimationResult() = 0;

		virtual bool ParseXML(tinyxml2::XMLElement* pelem) = 0;
		virtual void Save(tinyxml2::XMLElement& elem) = 0;
		virtual bool ParseLua(const fastbird::LuaObject& compTable) = 0;
		virtual float GetTextBottomGap() const = 0;

		virtual IWinBase* WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const = 0;
		virtual IWinBase* WinBaseWithTabOrder(unsigned tabOrder) const = 0;
		virtual void GatherTabOrder(VectorMap<unsigned, IWinBase*>& winbases) const = 0;
		virtual void TabPressed() = 0;

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
		virtual void RefreshScissorRects() = 0;

		virtual void SetEnable(bool enable) = 0;
		virtual bool GetEnable() const = 0;

		// usually don't need to use this functions
		// coordinates are decided by functions like SetNPos():for relative or SetPos() for absolute.
		virtual void SetUseAbsXPos(bool use) = 0;
		virtual void SetUseAbsYPos(bool use) = 0;
		virtual bool GetUseAbsXPos() const = 0;
		virtual bool GetUseAbsYPos() const = 0;
		virtual void SetUseAbsSize(bool use) = 0;
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
		virtual const char* GetMsgTranslationUnit() const = 0;
		virtual int GetTabOrder() const = 0;
		virtual void GetBiggestTabOrder(int& curBiggest) const = 0;

		virtual void SetSaveNameCheck(bool set) = 0;
		virtual bool GetSaveNameCheck() const = 0;

		// runtime child will not be saved.
		virtual void SetRuntimeChild(bool runtime) = 0;
		virtual bool IsRuntimeChild() const = 0;
		// ghost child will not test to focus
		virtual void SetGhost(bool ghost) = 0;
		virtual bool GetGhost() const = 0;

		virtual float GetContentHeight() const = 0;

		virtual bool IsKeyboardFocused() const = 0;

		virtual void TriggerRedraw() = 0;

		virtual void SetGatheringException() = 0;
		virtual bool GetGatheringException() const = 0;

		virtual void SetEvent(UIEvents::Enum e, const char* luaFuncName) = 0;
		virtual const char* GetEvent(UIEvents::Enum e) = 0;

		virtual bool GetNoMouseEvent() const = 0;
		virtual bool GetNoMouseEventAlone() const = 0;
		virtual void SetSpecialOrder(int specialOrder) = 0;

		virtual void RecreateBorders() = 0;

	protected:
		virtual void NotifySizeChange() = 0;
		virtual void OnPosChanged(bool anim) = 0;
		virtual void OnSizeChanged() = 0;
		virtual void OnEnableChanged() = 0;
		virtual void OnChildHasDragged()= 0;
	};
}