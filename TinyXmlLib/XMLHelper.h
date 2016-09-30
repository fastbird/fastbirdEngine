/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 Original code is from tinyxml2 library : http://www.grinninglizard.com/tinyxml2/
 -----------------------------------------------------------------------------
*/

#pragma once
#include "FBCommonHeaders/Types.h"
#include "tinyxml2.h"
#include <string>
namespace fb {
	tinyxml2::XMLElement* FindElem(tinyxml2::XMLHandle elem, const StringVector& path);
	tinyxml2::XMLElement* FindElem(tinyxml2::XMLHandle elem, const char* path);
	std::string GetStringAttribute(tinyxml2::XMLElement* elem, const char* key, bool& found);
	int GetIntAttribute(tinyxml2::XMLElement* elem, const char* key, int def = 0);
	bool HasElem(tinyxml2::XMLElement* elem, const char* key);
	tinyxml2::XMLElement* GetOrCreateChild(tinyxml2::XMLElement* elem, const char* key);
	tinyxml2::XMLElement* CreateChild(tinyxml2::XMLElement* elem, const char* key);
	void AppendText(tinyxml2::XMLElement* elem, const char* key, const char* content);
	void AppendTextArray(tinyxml2::XMLElement* elem, const char* key, const StringVector& strings);
	void SetText(tinyxml2::XMLElement* elem, const char* content);
}
