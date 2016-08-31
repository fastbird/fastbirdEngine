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

#include "XMLHelper.h"
#include "FBCommonHeaders/Types.h"
#include "FBStringLib/StringLib.h"
#include "FBDebugLib/DebugLib.h"

using namespace tinyxml2;

namespace fb {

	XMLElement* FindElem(XMLHandle elem, const StringVector& path) {
		for (auto& p : path) {
			if (p[0] == '@')
				break;
			elem = elem.FirstChildElement(p.c_str());
		}
		return elem.ToElement();
	}

	tinyxml2::XMLElement* FindElem(tinyxml2::XMLHandle elem, const char* path) {
		auto splitted = fb::Split(path, "/");
		return fb::FindElem(elem, splitted);
	}

	std::string GetStringAttribute(tinyxml2::XMLElement* elem, const char* key, bool& found) {
		found = false;
		if (!elem) {
			Logger::Log(FB_ERROR_LOG_ARG, "No elem");
			return{};
		}
		StringVector path = Split(key, "/");
		XMLElement* curElem = fb::FindElem(elem, path);
		if (!curElem) {
			/*Logger::Log(FB_ERROR_LOG_ARG,
				FormatString("Cannot find elem(%s)", key).c_str());*/
			return{};
		}
		auto sz = curElem->Attribute(path.back().c_str() + 1);
		if (!sz) {
			/*Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find int att(%s)",
				path.back().c_str() + 1).c_str());*/
		}
		else {
			found = true;
			return sz;
		}
		return{};
	}

	int GetIntAttribute(tinyxml2::XMLElement* elem, const char* key, int def) {
		if (!elem) {
			Logger::Log(FB_ERROR_LOG_ARG, "No elem");
			return def;
		}
		StringVector path = Split(key, "/");
		XMLElement* curElem = fb::FindElem(elem, path);
		int value;
		if (!curElem) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find elem(%s)", key).c_str());
			return def;
		}

		if (curElem->QueryIntAttribute(path.back().c_str() + 1, &value) == XML_NO_ERROR)
			return value;
		else {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find int att(%s)",
				path.back().c_str() + 1).c_str());
		}
		return def;
	}

	bool HasElem(tinyxml2::XMLElement* elem, const char* key) {
		class XMLFinder : public XMLVisitor {
		public:
			std::string mKey;
			bool mFound;
			XMLFinder(const char* key)
				:mKey(key)
				, mFound(false)
			{

			}
			bool VisitEnter(const XMLElement& elem/*element*/, const XMLAttribute* /*firstAttribute*/) {
				if (elem.Name() && mKey == elem.Name()) {
					mFound = true;
					return false;
				}
				return true;
			}
		};
		XMLFinder finder(key);
		elem->Accept(&finder);
		return finder.mFound;
	}

	tinyxml2::XMLElement* GetOrCreateChild(tinyxml2::XMLElement* elem, const char* key) {
		auto celem = elem->FirstChildElement(key);
		if (!celem) {
			celem = elem->GetDocument()->NewElement(key);
			elem->InsertEndChild(celem);
		}
		return celem;
	}

	tinyxml2::XMLElement* CreateChild(tinyxml2::XMLElement* elem, const char* key) {
		auto sp = Split(key, "/");
		auto e = elem;
		for (auto s : sp) {
			auto ce = e->GetDocument()->NewElement(s.c_str());
			e->InsertEndChild(ce);
			e = ce;
		}
		return e;
	}

	void AppendText(tinyxml2::XMLElement* elem, const char* key, const char* content) {
		auto celem = elem->GetDocument()->NewElement(key);
		elem->InsertEndChild(celem);
		SetText(celem, content);
	}

	void AppendTextArray(tinyxml2::XMLElement* elem, const char* key, const StringVector& strings) {
		for (auto& str : strings) {
			auto celem = elem->GetDocument()->NewElement(key);
			elem->InsertEndChild(celem);
			SetText(celem, str.c_str());
		}
	}

	void SetText(tinyxml2::XMLElement* elem, const char* content)
	{
		auto textElem = elem->GetDocument()->NewText(content);
		elem->InsertEndChild(textElem);

	}
}