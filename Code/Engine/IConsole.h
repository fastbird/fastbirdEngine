#pragma once
#include <functional>
#include <CommonLib/SmartPtr.h>
#include <CommonLib/StringUtils.h>
#include <Engine/EngineCommand.h>
#include <sstream>

#define REGISTER_CVAR(name, def, category, desc) \
	CVar* pCVar##name = FB_NEW(CVar)(#name, def, name, category, desc); \
	mCVars.push_back(pCVar##name);\
	gFBEnv->pConsole->RegisterVariable(pCVar##name);

#define REGISTER_CC(p) \
	mCommands.push_back(p);\
	gFBEnv->pConsole->RegisterCommand(p);

namespace fastbird
{
	class StdOutRedirect;
	enum CVAR_CATEGORY
	{
		CVAR_CATEGORY_SERVER,
		CVAR_CATEGORY_CLIENT,
	};

	enum CVAR_TYPE
	{
		CVAR_TYPE_INT,
		CVAR_TYPE_FLOAT,
		CVAR_TYPE_STRING
	};

	struct CVar
	{
		CVar(const char* _name, const int _def, int& _storage,
			CVAR_CATEGORY _category, const std::string& _desc)
			: mName(_name), mCategory(_category), mDesc(_desc)
			, mStorage(&_storage)
			, mType(CVAR_TYPE_INT)
		{
			ToLowerCase(mName);
			_storage = _def;	
		}

		CVar(const char* _name, const float _def, float& _storage,
			CVAR_CATEGORY _category, const std::string& _desc)
			: mName(_name), mCategory(_category), mDesc(_desc)
			, mStorage(&_storage)
			, mType(CVAR_TYPE_FLOAT)
		{
			ToLowerCase(mName);
			_storage = _def;	
		}

		// up to three characters.
		CVar(const char* _name, const std::string& _def, std::string& _storage,
			CVAR_CATEGORY _category, const std::string& _desc)
			: mName(_name), mCategory(_category), mDesc(_desc)
			, mStorage(&_storage)
			, mType(CVAR_TYPE_STRING)
		{
			ToLowerCase(mName);
			_storage = _def;
		}

		int GetInt() const
		{
			assert(mType == CVAR_TYPE_INT);
			return *(int*)mStorage;
		}

		float GetFloat() const
		{
			assert(mType == CVAR_TYPE_FLOAT);
			return *(float*)mStorage;
		}

		std::string& GetString() const
		{
			assert(mType == CVAR_TYPE_STRING);
			return *(std::string*)mStorage;
		}

		void SetData(const std::string& data)
		{
			switch(mType)
			{
			case CVAR_TYPE_INT:
				*(int*)mStorage = StringConverter::parseInt(data);
				break;

			case CVAR_TYPE_FLOAT:
				*(float*)mStorage = StringConverter::parseReal(data);
				break;

			case CVAR_TYPE_STRING:
				*(std::string*)mStorage = data;
				break;
			}
		}

		std::string GetData() const
		{
			std::stringstream ss;
			switch(mType)
			{
			case CVAR_TYPE_INT:				
				ss << (*(int*)mStorage);
				return ss.str();
				break;

			case CVAR_TYPE_FLOAT:
				ss << (*(float*)mStorage);
				return ss.str();
				break;

			case CVAR_TYPE_STRING:
				return (*(std::string*)mStorage);
				break;
			}
			assert(0);
			return ss.str();
		}

		// make sure lower case.
		std::string mName;
		CVAR_CATEGORY mCategory;
		CVAR_TYPE mType;
		void* mStorage;
		std::string mDesc;
	};

	typedef std::function<void(StringVector&)> CFunc;
	//--------------------------------------------------------------------------
	struct ConsoleCommand
	{
		ConsoleCommand(const std::string& name, CFunc f,
			const std::string& desc)
			: mName(name), mFunc(f), mDesc(desc)
		{
			ToLowerCase(mName);
		}

		// make sure use only low case.
		std::string mName;
		CFunc mFunc;
		std::string mDesc;
	};

	class ICVarListener
	{
	public:
		virtual bool OnChangeCVar(CVar* pCVar) = 0;
	};

	//--------------------------------------------------------------------------
	class IConsole : public ReferenceCounter
	{
	public:
		static IConsole* CreateConsole();

		virtual bool Init() = 0;
		virtual void RegisterCommand(ConsoleCommand* pCom) = 0;
		virtual void UnregisterCommand(ConsoleCommand* pCom) = 0;
		virtual void RegisterVariable(CVar* cvar) = 0;
		virtual void UnregisterVariable(CVar* cvar) = 0;
		virtual void AddCandidatesTo(const char* parent, const StringVector& candidates) = 0;
		virtual void Log(const char* szFmt, ...) = 0;
		virtual void ProcessCommand(const char* command) = 0;
		virtual void ToggleOpen() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;
		virtual void AddListener(ICVarListener* pListener) = 0;
		virtual void RemoveListener(ICVarListener* pListener) = 0;
		virtual EngineCommand* GetEngineCommand() = 0;
		virtual void RegisterStdout(StdOutRedirect* p) = 0;
		virtual void Clear() = 0;
	};

}