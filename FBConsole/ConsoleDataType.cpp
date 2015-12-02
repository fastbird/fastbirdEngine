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

#include "stdafx.h"
#include "ConsoleDataType.h"
#include "FBStringLib/StringLib.h"
#include "FBStringLib/StringConverter.h"
#include "FBStringMathLib/StringMathConverter.h"
#include "FBMathLib/Vec2I.h"
#include <sstream>
using namespace fb;
CVar::CVar(const char* _name, const int _def, int& _storage,
	CVAR_CATEGORY _category, const std::string& _desc)
	: mName(_name), mCategory(_category), mDesc(_desc)
	, mStorage(&_storage)
	, mType(CVAR_TYPE_INT)
{
	ToLowerCase(mName);
	_storage = _def;
}

CVar::CVar(const char* _name, const float _def, float& _storage,
	CVAR_CATEGORY _category, const std::string& _desc)
	: mName(_name), mCategory(_category), mDesc(_desc)
	, mStorage(&_storage)
	, mType(CVAR_TYPE_REAL)
{
	ToLowerCase(mName);
	_storage = _def;
}

// up to three characters.
CVar::CVar(const char* _name, const std::string& _def, std::string& _storage,
	CVAR_CATEGORY _category, const std::string& _desc)
	: mName(_name), mCategory(_category), mDesc(_desc)
	, mStorage(&_storage)
	, mType(CVAR_TYPE_STRING)
{
	ToLowerCase(mName);
	_storage = _def;
}

CVar::CVar(const char* _name, const Vec2I& _def, Vec2I& _storage,
	CVAR_CATEGORY _category, const std::string& _desc)
	: mName(_name), mCategory(_category), mDesc(_desc)
	, mStorage(&_storage)
	, mType(CVAR_TYPE_VEC2I)
{
	ToLowerCase(mName);
	_storage = _def;
}

int CVar::GetInt() const
{
	assert(mType == CVAR_TYPE_INT);
	return *(int*)mStorage;
}

float CVar::GetFloat() const
{
	assert(mType == CVAR_TYPE_REAL);
	return *(float*)mStorage;
}

std::string& CVar::GetString() const
{
	assert(mType == CVAR_TYPE_STRING);
	return *(std::string*)mStorage;
}
Vec2I& CVar::GetVec2I() const
{
	assert(mType == CVAR_TYPE_VEC2I);
	return *(Vec2I*)mStorage;
}

void CVar::SetData(const std::string& data)
{
	switch (mType)
	{
	case CVAR_TYPE_INT:
		*(int*)mStorage = StringConverter::ParseInt(data);
		break;

	case CVAR_TYPE_REAL:
		*(Real*)mStorage = StringConverter::ParseReal(data);
		break;

	case CVAR_TYPE_STRING:
		*(std::string*)mStorage = data;
		break;

	case CVAR_TYPE_VEC2I:
		*(Vec2I*)mStorage = StringMathConverter::ParseVec2I(data);
		break;

	default:
		assert(0 && "Not implemented");
	}
}

std::string CVar::GetData() const
{
	std::stringstream ss;
	switch (mType)
	{
	case CVAR_TYPE_INT:
		ss << (*(int*)mStorage);
		return ss.str();
		break;

	case CVAR_TYPE_REAL:
		ss << (*(float*)mStorage);
		return ss.str();
		break;

	case CVAR_TYPE_STRING:
		return (*(std::string*)mStorage);
		break;

	case CVAR_TYPE_VEC2I:
		ss << StringMathConverter::ToString(*(Vec2I*)mStorage);
		return ss.str();
		break;
	}
	assert(0);
	return ss.str();
}

//---------------------------------------------------------------------------
ConsoleCommand::ConsoleCommand(const std::string& name, CFunc f,
	const std::string& desc)
	: mName(name), mFunc(f), mDesc(desc)
{
	ToLowerCase(mName);
}