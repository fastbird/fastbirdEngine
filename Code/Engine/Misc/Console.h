#pragma once

#include <Engine/IConsole.h>

struct lua_State;
namespace fastbird
{
	class CandidatesData;
	class Console : public IConsole, public IInputListener
	{
	public:
		Console();
		virtual ~Console();

		//-------------------------------------------------------------------------
		// IConsole
		virtual bool Init();
		virtual void RegisterCommand(ConsoleCommand* pCom);
		virtual void UnregisterCommand(ConsoleCommand* pCom);
		virtual void RegisterVariable(CVar* cvar);
		virtual void UnregisterVariable(CVar* cvar);
		virtual void AddCandidatesTo(const char* parent, const StringVector& candidates);
		virtual void Log(const char* szFmt, ...);
		virtual void ProcessCommand(const char* command);
		virtual void ToggleOpen();
		virtual void Render();
		virtual EngineCommand* GetEngineCommand() { return mEngineCommand; }
		virtual void AddListener(ICVarListener* pListener);
		virtual void RemoveListener(ICVarListener* pListener);

		//-------------------------------------------------------------------------
		// IInputListener
		virtual void OnInput(IMouse* pMouse, IKeyboard* pKeyboard);
		virtual void EnableInputListener(bool enable);
		virtual bool IsEnabledInputLIstener() const;

		void AutoCompletion();
		bool IsValidCharForInput(unsigned int chr);
		void OnCVarChanged(CVar* cvar);

	
	private:

		void Highlighting(bool shiftkey);
		bool IsHighlighting() const { return mHighlightStart!=-1; }
		void StartHighlighting()
		{ 
			assert(!IsHighlighting()); 
			mHighlightStart = mCursorPos;
		}
		void EndHighlighting() { mHighlightStart = -1; }
		void EndAutoCompletion();

		void GetNextHistory();
		void GetPrevHistory();

	private:
		int mCursorPos;
		const int mCursorWidth;
		int mHighlightStart;
		std::string mInputString;
		std::wstring mInputStringw;
		StringVector mBuffer;
		WStringVector mBufferw; // display buffer
		FB_CRITICAL_SECTION mBufferwCS;

		int mBufferBtmLine; // bottom line
		int mLines;
		bool mOpen;
		const int mLineGap;
		int mHeight;
		std::wstring mPrompt;
		int mPromptStart;
		Vec2I mInputPosition;
		typedef std::vector<CVar*> CVARS;
		CVARS mCVars;

		typedef std::vector<ConsoleCommand*> COMMANDS;
		COMMANDS mCommands;

		CandidatesData* mCandiData;
		bool mACMode; // AutoCompletionMode
		StringVector mCandidates;
		std::string mInputStringBackup; // auto completed input
		int mCandiDepth;
		int mCandiIndex;

		StringVector mHistory;
		int mHistoryIndex;

		SmartPtr<EngineCommand> mEngineCommand;

		std::vector<ICVarListener*> mCVarListeners;

	};
}