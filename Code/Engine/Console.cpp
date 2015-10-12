#include <Engine/StdAfx.h>
#include <Engine/Console.h>
#include <Engine/CandidatesData.h>
#include <Engine/EngineCommand.h>
#include <Engine/GlobalEnv.h>
#include <Engine/ScriptSystem.h>
#include <Engine/Renderer.h>
#include <CommonLib/StdOutRedirect.h>
#include <CommonLib/ClipboardData.h>

namespace fastbird
{
//--------------------------------------------------------------------------
IConsole* IConsole::CreateConsole()
{
	Console* pCon = FB_NEW(Console);
	return pCon;
}

const char* cacheFile = "consoleCache.tmp";
//--------------------------------------------------------------------------
Console::Console()
	: mCursorPos(0)
	, mCursorWidth(10)
	, mLines(15)
	, mOpen(false)
	, mLineGap(2)
	, mHighlightStart(-1)
	, mACMode(false), mCandiDepth(0), mCandiIndex(0)
	, mBufferBtmLine(0)
	, mLuaMode(false)
	, mStdOutRedirect(0)
	, mHistoryIndex(0)
	, mHistoryIndexBackup(0)
{
	assert(gFBEnv->pConsole == 0);
	gFBEnv->pConsole = this;

	gFBEnv->pEngine->AddInputListener(this,
		fastbird::IInputListener::INPUT_LISTEN_PRIORITY_CONSOLE, 0);

	mCandiData = FB_NEW(CandidatesData);
	mEngineCommand = FB_NEW(EngineCommand);

	std::ifstream file(cacheFile);
	if (file.is_open())
	{
		do {
			char buf[256] = { 0 };
			file.getline(buf, 256);
			if (strlen(buf) != 0)
			{
				mHistory.push_back(buf);
				mValidHistory.push_back(true);
			}
		} while (!file.eof());
		file.close();
		mHistoryIndex = mHistory.size();
	}
}

//--------------------------------------------------------------------------
Console::~Console()
{
	FB_DELETE(mCandiData);	
}

void Console::FinishSmartPtr(){
	FB_DELETE(this);
}

/*
//--------------------------------------------------------------------------
void Input(void* arg)
{
	while(gFBEnv && !gFBEnv->mExiting)
	{
		std::string strInput;
		std::getline(std::cin, strInput);
		
		gFBEnv->pScriptSystem->ExecuteLua(strInput);
	}
}

//--------------------------------------------------------------------------
void Output(void* arg)
{
	while(gFBEnv && !gFBEnv->mExiting)
	{
		if (!gOutputs.empty())
		{
			WaitForSingleObject(g_hMutexForOutputBuffer, INFINITE);
			std::string msg = gOutputs.front();
			gOutputs.pop();
			ReleaseMutex(g_hMutexForOutputBuffer);

			std::cout << msg << std::endl;
		}
	}
}*/

//--------------------------------------------------------------------------
bool Console::Init()
{
	// calc background size
	mHeight = (sFontSize + mLineGap) * mLines;

	mPrompt = AnsiToWide(">", 1);
	mPromptStart = 2;
	mInputPosition = Vec2I(20, mHeight - mLineGap);

	return true;
}

//--------------------------------------------------------------------------
void Console::RegisterCommand(ConsoleCommand* pCom)
{
	for (const auto& c : mCVars)
	{
		if (c->mName == pCom->mName)
		{
			Error("The same name of CVar already exists.");
			return;
		}
	}

	for (const auto& c : mCommands)
	{
		if (c->mName == pCom->mName)
		{
			Error("The same name of CFunc already exists.");
		}
	}
	mCandiData->AddCandidate(pCom->mName.c_str());
	mCommands.push_back(pCom);
}

//--------------------------------------------------------------------------
void Console::UnregisterCommand(ConsoleCommand* pCom)
{
	mCommands.erase(std::remove(mCommands.begin(), mCommands.end(), 
		pCom), mCommands.end());
}

//--------------------------------------------------------------------------
void Console::RegisterVariable(CVar* cvar)
{
	// check the name;
	for (const auto& c : mCVars)
	{
		if (c->mName == cvar->mName || c == cvar)
		{
			Error("The same CVar already exists.");
			return;
		}
	}

	for (const auto& c : mCommands)
	{
		if (c->mName == cvar->mName)
		{
			Error("The same name of CFunc already exists.");
		}
	}

	mCandiData->AddCandidate(cvar->mName.c_str());

	mCVars.push_back(cvar);
}

//--------------------------------------------------------------------------
void Console::UnregisterVariable(CVar* cvar)
{
	mCVars.erase(std::remove(mCVars.begin(), mCVars.end(), cvar), mCVars.end());
}

void Console::AddCandidatesTo(const char* parent, const StringVector& candidates)
{
	mCandiData->AddCandidatesTo(parent, candidates);
}

//--------------------------------------------------------------------------
void Console::Log(const char* szFmt, ...)
{
	LOCK_CRITICAL_SECTION lock(mBufferwCS);
	char buf[4096];

	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 4096, szFmt, args);
	va_end(args);

	std::cerr << buf << std::endl;

	std::wstring strw = AnsiToWide(buf, strlen(buf));
	if (gFBEnv->pRenderer && gFBEnv->pRenderer->GetFont())
	{
		IFont* pFont = gFBEnv->pRenderer->GetFont();
		int textWidth = (int)pFont->GetTextWidth((char*)strw.c_str());
		const auto& size = gFBEnv->_pInternalRenderer->GetMainRTSize();
		int consoleWidth = size.x;
		if (textWidth > consoleWidth)
		{
			strw = pFont->StripTags(strw.c_str());
			int numCharInRow = 20;
			int len = strw.length();
			int count = 0;
			int curLine = 0;
			WStringVector lines;
			int start = 0;
			int end = 1;
			while (end<len)
			{
				int textWidth = (int)pFont->GetTextWidth((char*)&strw[start], (end-start)*2);
				if (textWidth>consoleWidth)
				{
					mBuffer.push_back(std::string(buf+start, end - start));
					start = end;
					end +=1;
				}
				else
				{
					end++;
				}
			}
			mBuffer.push_back(std::string(buf+start));
		}
		else
		{
			mBuffer.push_back(buf);
		}
	}
	else
	{
		// no renderer
		mBuffer.push_back(buf);
	}

	mBufferw.clear();
	auto rit = mBuffer.rbegin(), ritEnd = mBuffer.rend();
	int remainedLined = mLines-1;
	for (; rit != ritEnd && remainedLined; rit++)
	{
		auto strVec = Split(*rit, "\n");
		auto rit2 = strVec.rbegin(), ritEnd2 = strVec.rend();
		for (; rit2 != ritEnd2 && remainedLined; rit2++)
		{
			if (!rit2->empty())
			{
				mBufferw.push_back(AnsiToWide(rit2->c_str()));
				remainedLined--;
			}
		}
		
	}
}

//--------------------------------------------------------------------------
void Console::ToggleOpen()
{
	mOpen = !mOpen;
	gFBEnv->pEngine->GetKeyboard()->ClearBuffer();
}

//--------------------------------------------------------------------------
void Console::Update()
{
	if (mStdOutRedirect)
	{
		char buf[1024];
		int len = mStdOutRedirect->GetBuffer(buf, 1024);
		if (len)
		{
			buf[1023] = 0;
			Log(buf);
		}
	}
}

//--------------------------------------------------------------------------
void Console::Render()
{
	if (!mOpen)
		return;
	IRenderer* pRenderer = gFBEnv->pRenderer;
	IFont* pFont = pRenderer->GetFont();
	pFont->SetHeight((float)sFontSize);
	const int lineHeight = (int)pFont->GetHeight();

	

	// Draw Background
	const auto& size = gFBEnv->_pInternalRenderer->GetMainRTSize();
	pRenderer->DrawQuad(Vec2I(0, 0), Vec2I(size.x, mHeight),
		Color::DarkGray);


	// DrawHighlight
	if (mHighlightStart != -1)
	{
		Vec2I highlightStartPos = mInputPosition;
		highlightStartPos.x += (int)pFont->GetTextWidth((char*)mInputStringw.c_str(),
			mHighlightStart * 2);
		highlightStartPos.y -= lineHeight;
		Vec2I highlightEndPos = mInputPosition;
		highlightEndPos.x += (int)pFont->GetTextWidth((char*)mInputStringw.c_str(),
			mCursorPos * 2);
		highlightEndPos.y -= lineHeight;

		if (mHighlightStart > mCursorPos)
		{
			std::swap(highlightStartPos, highlightEndPos);
		}

		highlightEndPos.y += lineHeight;

		pRenderer->DrawQuad(highlightStartPos, highlightEndPos - highlightStartPos,
			Color::Gray);
	}
	
	// Draw Cursor
	Vec2I cursorPos = mInputPosition;
	cursorPos.x += (int)pFont->GetTextWidth((char*)mInputStringw.c_str(), mCursorPos * 2);
	pRenderer->DrawQuad(cursorPos, Vec2I(mCursorWidth, 2), Color::Yellow);

	// Draw prompt
	pFont->PrepareRenderResources();
	pFont->SetRenderStates();
	pFont->Write((float)mPromptStart, (float)mInputPosition.y, 0.f, Color::White, 
		(char*)mPrompt.c_str(), -1, FONT_ALIGN_LEFT);

	// Draw Input String
	pFont->Write((float)mInputPosition.x, (float)mInputPosition.y, 0.f, Color::White, 
			(char*)mInputStringw.c_str(), -1, FONT_ALIGN_LEFT);

	

	// Draw Buffer
	WStringVector bufferwRender(mBufferw.size());
	{
		LOCK_CRITICAL_SECTION lock(mBufferwCS);
		std::copy(mBufferw.begin(), mBufferw.end(), bufferwRender.begin());
	}
	
	int bufferDrawPosY = mInputPosition.y - lineHeight - mLineGap;
	auto it = bufferwRender.begin();
	auto itEnd = bufferwRender.end();
	for (; it < itEnd; it++)
	{
		unsigned numLineFeed = std::count(it->begin(), it->end(), L'\n');
		pFont->Write((float)mInputPosition.x, (float)bufferDrawPosY, 0.f,
			Color::Gray, (char*)it->c_str(), -1, FONT_ALIGN_LEFT);
		bufferDrawPosY -= lineHeight + mLineGap;
	}
	pFont->SetBackToOrigHeight();
	


}

void Console::OnInput(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mOpen)
		return;

	if (unsigned int chr = keyboard->GetChar())
	{
		if (chr == 22) // Synchronous idle - ^V
		{
			keyboard->PopChar();
			std::string data = GetClipboardDataAsString(gFBEnv->pEngine->GetMainWndHandle());
			if (!data.empty())
			{
				auto insertionPos = mInputString.begin() + mCursorPos;
				mInputString.insert(insertionPos, data.begin(), data.end());
				mCursorPos += data.size();
			}
		}
		else
		{
			switch (chr)
			{
			case VK_BACK:
			{
				keyboard->PopChar();
							if (IsHighlighting() && !mInputString.empty())
							{
								auto it = mInputString.begin() + mCursorPos;
								auto end = mInputString.begin() + mHighlightStart;
								if (mCursorPos > mHighlightStart)
								{
									std::swap(it, end);
								}
								mCursorPos = std::distance(mInputString.begin(), it);
								mInputString.erase(it, end);
								EndHighlighting();
							}
							else if (mCursorPos > 0 && !mInputString.empty())
							{
								mInputString.erase(mInputString.begin() + mCursorPos - 1);
								mCursorPos--;
							}
							EndAutoCompletion();
			}
				break;
			case VK_TAB:
			{
				keyboard->PopChar();
				AutoCompletion();
			}
				break;
			case VK_RETURN:
			{
				keyboard->PopChar();
				if (!mInputString.empty())
				{
					ProcessCommand(mInputString.c_str());
					mInputString.clear();
					mCursorPos = 0;
					EndHighlighting();
					EndAutoCompletion();
				}
			}
				break;
			default:
			{
					   if (IsValidCharForInput(chr))
					   {
						   keyboard->PopChar();
						   if (IsHighlighting())
						   {
							   if (!mACMode)
							   {
								   auto it = mInputString.begin() + mCursorPos;
								   auto end = mInputString.begin() + mHighlightStart;
								   if (mCursorPos > mHighlightStart)
								   {
									   std::swap(it, end);
								   }
								   mCursorPos = std::distance(mInputString.begin(), it);
								   // delete selected string
								   mInputString.erase(it, end);
							   }
							   EndHighlighting();
						   }

						   EndAutoCompletion();

						   mInputString.insert(mInputString.begin() + mCursorPos, chr);
						   mCursorPos++;
					   }
			}
			}
		}
	}

	if (keyboard->IsKeyPressed(VK_HOME))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		mCursorPos = 0;
	}
	else if (keyboard->IsKeyPressed(VK_END))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));	
		mCursorPos = mInputString.size();
	}
	else if (keyboard->IsKeyPressed(VK_DELETE))
	{
		if (!mInputString.empty())
		{
			if (IsHighlighting())
			{
				auto it = mInputString.begin() + mCursorPos;
				auto end = mInputString.begin() + mHighlightStart;
				if (mCursorPos > mHighlightStart)
				{
					std::swap(it, end);
				}
				mCursorPos = std::distance(mInputString.begin(), it);
				mInputString.erase(it, end);
				EndHighlighting();
			}
			else
			{
				mInputString.erase(mInputString.begin()+mCursorPos);
			}
		}
		EndAutoCompletion();
	}
	else if (keyboard->IsKeyPressed(VK_LEFT))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		if (mCursorPos>0)
		{
			mCursorPos--;
		}
		EndAutoCompletion();
	}
	else if (keyboard->IsKeyPressed(VK_RIGHT))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		if (mCursorPos < (int)mInputString.size())
		{
			mCursorPos++;
		}
		EndAutoCompletion();
	}
	else if (keyboard->IsKeyPressed(VK_UP))
	{
		GetNextHistory();

	}
	else if (keyboard->IsKeyPressed(VK_DOWN))
	{
		GetPrevHistory();		
	}

	mInputStringw = AnsiToWide(mInputString.c_str(), mInputString.size());

	keyboard->Invalidate();
	mouse->Invalidate();
}

void Console::EnableInputListener(bool enable)
{
	//mActivated
}

bool Console::IsEnabledInputLIstener() const
{
	return mOpen;
}

void Console::ProcessCommand(const char* command)
{
	if (!command || strlen(command)==0 || strlen(command)>512)
	{
		Error("Invalid console command");
		return;
	}

	if (strcmp("%", command) == 0)
	{
		mLuaMode = true;
		mPrompt = AnsiToWide("%", 1);
		Log("Start Lua interactive mode.");
		return;
	}
	else if (strcmp(">", command)==0)
	{
		mLuaMode = false;
		mPrompt = AnsiToWide(">", 1);
		Log("Start Game command mode.");
		return;
	}

	bool prefixFound = false;
	const char* found;
	if ((found = strchr(command, '%')) && command == found)
	{
		mLuaMode = true;
		mPrompt = AnsiToWide("%", 1);
		prefixFound = true;
	}
	else if ((found = strchr(command, '>')) && command == found)
	{
		mLuaMode = false;
		mPrompt = AnsiToWide(">", 1);
		prefixFound = true;
	}
	char buf[512];
	if (!prefixFound)
	{
		sprintf_s(buf, "%s%s", mLuaMode ? "%" : ">", command);
	}
	else
	{
		sprintf_s(buf, "%s", command);
	}

	bool pushed = false;
	if (mHistory.empty() || mHistory.back() != buf)
	{
		for (auto it = mHistory.begin(); it != mHistory.end();)
		{
			if (*it == buf)
			{
				unsigned dist = std::distance(mHistory.begin(), it);
				it = mHistory.erase(it);
				mValidHistory.erase(mValidHistory.begin() + dist);
			}
			else
			{
				++it;
			}
		}
		DeleteValuesInVector(mHistory, std::string(buf));
		mHistory.push_back(buf);
		mHistoryIndex = mHistory.size();
		pushed = true;
	}
	else
	{
		mHistoryIndex = mHistoryIndexBackup;
	}

	if (prefixFound)
		command = buf + 1;
		
	bool processed = false;
	if (mLuaMode)
	{
		std::string newstring = std::string("  ") + command;
		Log(newstring.c_str());
		processed = gFBEnv->pScriptSystem->ExecuteLua(command);
		if (pushed)
			mValidHistory.push_back(processed);
		if (processed)
		{			
			SaveHistoryToDiskCache();
		}
			
		return;
	}
	StringVector words = StringConverter::parseStringVector(command);
	if (words.empty())
		return;

	auto it = words.begin(), itEnd = words.end();
	for (; it!=itEnd; it++)
	{
		ToLowerCase(*it);
	}

	
	//find command
	{
		for (const auto& c : mCommands)
		{
			if (_stricmp(c->mName.c_str(), words[0].c_str())==0)
			{
				if (c->mFunc)
				{
					c->mFunc(words);
					processed = true;
				}
			}
		}
	}

	// find cvar
	{
		for (const auto& c : mCVars)
		{
			if (_stricmp(c->mName.c_str(), words[0].c_str())==0)
			{
				size_t numWords = words.size();
				if (numWords==2)
				{
					c->SetData(words[1]);
					OnCVarChanged(c);
				}
				this->Log("%s %s", c->mName.c_str(), c->GetData().c_str());
				processed = true;
			}
		}
	}
	if (pushed)
		mValidHistory.push_back(processed);

	if (processed)
		SaveHistoryToDiskCache();
}

void Console::SaveHistoryToDiskCache()
{
	if (!mHistory.empty())
	{
		std::ofstream file(cacheFile);
		if (file.is_open())
		{
			assert(mHistory.size() == mValidHistory.size());
			auto rit = mValidHistory.rbegin(), rend = mValidHistory.rend();
			int index = mValidHistory.size();
			int count = 0;
			for (; rit != rend && count < 10; rit++)
			{
				if (*rit)
				{
					++count;
				}
				--index;
				
			}
			for (unsigned i = index; i < mHistory.size(); i++)
			{
				if (mValidHistory[i])
					file << mHistory[i] << std::endl;
			}
			file.close();
		}
	}
}

void Console::AutoCompletion()
{
	if (!mACMode)
	{
		mCandidates = mCandiData->GetCandidates(mInputString.c_str(), mCandiDepth);
		if (mCandidates.empty())
			return;

		mACMode = true;
		mCandiIndex = 0;
		mInputStringBackup = mInputString;
	}
	else
	{
		mCandiIndex++;
		if (mCandiIndex >= (int)mCandidates.size())
		{
			mCandiIndex = 0;
		}
	}

	if (!mCandidates.empty())
	{	
		if (mCandiDepth==0)
		{
			// parent
			mInputString = mCandidates[mCandiIndex];
		}
		else
		{
			// child
			size_t loc = mInputStringBackup.find(' ');
			assert(loc != std::string::npos);
			loc++;
			mInputString.assign(mInputStringBackup, 0, loc);
			mInputString.append(mCandidates[mCandiIndex]);
		}
		int aclen = mInputString.length();
		int inlen = mInputStringBackup.length();
		mCursorPos = inlen;
		EndHighlighting();
		StartHighlighting();
		mCursorPos = aclen;

		mInputStringw = AnsiToWide(mInputString.c_str(), mInputString.size());
	}
}

bool Console::IsValidCharForInput(unsigned int chr)
{
	if (chr==0x60) // `
		return false;

	return true;
}

void Console::Highlighting(bool shiftkey)
{
	if (shiftkey)
	{
		if (!IsHighlighting())
		{
			StartHighlighting();
		}
	}
	else
	{
		EndHighlighting();
	}
}

void Console::EndAutoCompletion()
{
	if (mACMode)
	{
		mACMode = false;
		mCursorPos = mInputString.size();
	}
}

void Console::GetNextHistory()
{
	if (mHistory.empty())
		return;

	mHistoryIndexBackup = mHistoryIndex;
	--mHistoryIndex;
	if (mHistoryIndex >= (int)mHistory.size() || mHistoryIndex<0)
	{
		mHistoryIndex = (int)mHistory.size()-1;
	}

	mInputString = mHistory[mHistoryIndex].c_str();
	mCursorPos = 0;
	EndHighlighting();
	StartHighlighting();
	mCursorPos = mInputString.size();
	mInputStringw = AnsiToWide(mInputString.c_str(), mInputString.size());		
}

void Console::GetPrevHistory()
{
	if (mHistory.empty())
		return;
	mHistoryIndexBackup = mHistoryIndex;
	++mHistoryIndex;
	if (mHistoryIndex >= (int)mHistory.size() || mHistoryIndex<0)
	{
		mHistoryIndex = 0;
	}

	mInputString = mHistory[mHistoryIndex].c_str();
	mCursorPos = 0;
	EndHighlighting();
	StartHighlighting();
	mCursorPos = mInputString.size();
	mInputStringw = AnsiToWide(mInputString.c_str(), mInputString.size());
}

void Console::AddListener(ICVarListener* pListener)
{
	assert(std::find(mCVarListeners.begin(), mCVarListeners.end(), pListener) == mCVarListeners.end());
	mCVarListeners.push_back(pListener);
}
void Console::RemoveListener(ICVarListener* pListener)
{
	mCVarListeners.erase(std::remove(mCVarListeners.begin(), mCVarListeners.end(), pListener),
		mCVarListeners.end());
}

void Console::OnCVarChanged(CVar* cvar)
{
	FB_FOREACH(it, mCVarListeners)
	{
		(*it)->OnChangeCVar(cvar);
	}
}

void Console::RegisterStdout(StdOutRedirect* p)
{
	mStdOutRedirect = p;
}

void Console::Clear()
{
	mBufferw.clear();
}

}