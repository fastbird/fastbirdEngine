/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#include <string>

#define FB_REGISTER_CVAR(name, def, category, desc) \
	if (Console::HasInstance())\
		Console::GetInstance().RegisterVariable(CVarPtr(new CVar(#name, def, name, category, desc)));

#define FB_REGISTER_CC(func, desc) \
	if (Console::HasInstance())\
		Console::GetInstance().RegisterCommand(ConsoleCommandPtr(new ConsoleCommand(#func, func, desc)));

namespace fb{
	class Vec2I;
	enum CVAR_CATEGORY
	{
		CVAR_CATEGORY_SERVER,
		CVAR_CATEGORY_CLIENT,
	};

	enum CVAR_TYPE
	{
		CVAR_TYPE_INT,
		CVAR_TYPE_REAL,
		CVAR_TYPE_STRING,
		CVAR_TYPE_VEC2I,
	};

	FB_DECLARE_SMART_PTR_STRUCT(CVar);
	struct FB_DLL_CONSOLE CVar
	{
		CVar(const char* _name, const int _def, int& _storage,
			CVAR_CATEGORY _category, const std::string& _desc);
		CVar(const char* _name, const float _def, float& _storage,
			CVAR_CATEGORY _category, const std::string& _desc);
		// up to three characters.
		CVar(const char* _name, const std::string& _def, std::string& _storage,
			CVAR_CATEGORY _category, const std::string& _desc);
		CVar(const char* _name, const Vec2I& _def, Vec2I& _storage,
			CVAR_CATEGORY _category, const std::string& _desc);		

		int GetInt() const;
		float GetFloat() const;
		std::string& GetString() const;
		Vec2I& GetVec2I() const;
		void SetData(const std::string& data);
		std::string GetData() const;

		// make sure lower case.
		std::string mName;
		CVAR_CATEGORY mCategory;
		CVAR_TYPE mType;
		void* mStorage;
		std::string mDesc;
	};

	typedef std::function<void(StringVector&)> CFunc;
	//--------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR_STRUCT(ConsoleCommand);
	struct FB_DLL_CONSOLE ConsoleCommand
	{
		ConsoleCommand(const std::string& name, CFunc f,
			const std::string& desc);

		// make sure use only low case.
		std::string mName;
		CFunc mFunc;
		std::string mDesc;
	};
}