#pragma once
#include <UI/Container.h>
namespace fastbird{
	class TabWindow: public Container
	{
		static const Vec2I sButtonSize;
		IWinBase* mMainWindow;
		std::vector<IWinBase*> mButtons;
		std::vector<IWinBase*> mWindows;
		unsigned mNumTabs;
		std::string mStrTabNames;
		unsigned mCurTabIndex;

	public:
		TabWindow();
		virtual ~TabWindow();

		virtual ComponentType::Enum GetType() const { return ComponentType::TabWindow; }
		virtual void OnCreated();
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly);
		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		virtual void Save(tinyxml2::XMLElement& elem);
		virtual bool SetVisible(bool show);
		void UpdateTabIndex(unsigned index);

	private:
		void BuildTabWnd(unsigned numTabs);
		void SetTabNames(const StringVector& names);
		void OnTabClicked(void* arg);
	};
}