#include <CommonLib/StdAfx.h>
#include "StringUtils.h"

namespace fastbird
{
//------------------------------------------------------------------------
char* StripRight(char* s)
{
	char* p = s + strlen(s);
	while ( p > s && isspace((unsigned char)(*--p)) )
		*p = '\0';
	return s;
}

//------------------------------------------------------------------------
char* StripLeft(char* s)
{
	while (*s && isspace((unsigned char)(*s)))
		s++;
	return (char*)s;
}

//------------------------------------------------------------------------
std::string StripBoth(const char* s)
{
	const char* b = s;
	const char* e = s + strlen(s);
	while (b != e && isspace(*b))
	{
		b++;
	}
	while (e != b && isspace(*(e-1)))
	{
		e--;
	}
	return std::string(b, e - b);
}

//------------------------------------------------------------------------
void StripExtension(char* s)
{
	size_t len = strlen(s);
	while(--len)
	{
		if (s[len]=='.')
		{
			s[len]=0;
			break;
		}
	}
}

std::string StripExtension(const char* s)
{
	const char* end = FindLastOf(s, '.');
	if (end)
		return std::string(s, end);
	else
		return std::string();
}

const char* FindLastOf(const char* s, char ch)
{
	int len = strlen(s);
	const char* it = s + len - 1;
	for (; it >= s; --it)
	{
		if (*it == ch)
		{
			return it;
		}
	}
	return 0;
}

std::string GetFileName(const char* s)
{
	int len = strlen(s);
	const char* nameStart = FindLastOf(s, '/');
	if (!nameStart)
	{
		nameStart = FindLastOf(s, '\\');
	}
	if (nameStart)
	{
		return std::string(nameStart + 1, s + len);
	}
	return std::string();
}

std::string GetFileNameWithoutExtension(const char* s)
{
	int len = strlen(s);
	const char* nameStart = FindLastOf(s, '/');
	if (!nameStart)
	{
		nameStart = FindLastOf(s, '\\');
	}
	if (nameStart)
	{
		const char* dot = FindLastOf(s, '.');
		return std::string(nameStart + 1, dot);
	}
	return std::string();
}

std::string GetDirectoryPath(const char* s)
{
	assert(s && strlen(s) != 0);
	std::string ret(s);
	size_t f = ret.find_last_of('/');
	if (f == std::string::npos)
	{
		f = ret.find_last_of('\\');
	}

	if (f != std::string::npos)
	{
		return ret.substr(0, f);
	}
	
	return ret;
}

//------------------------------------------------------------------------
const char* StripPath(const char* s)
{
	size_t len = strlen(s);
	while(--len)
	{
		if (s[len]=='/')
		{
			return &s[len+1];
		}
	}
	return s;
}

//------------------------------------------------------------------------
const char* GetFileExtension(const char* s)
{
	size_t len = strlen(s);
	while(--len)
	{
		if (s[len]=='.')
		{
			return &s[len+1];
		}
	}

	return "";
}

//------------------------------------------------------------------------
bool CheckExtension(const char* filename, const char* extension)
{
	const char* szExtension = GetFileExtension(filename);
	if (szExtension==0)
		return false;

	return _stricmp(szExtension, extension)==0;
}

//------------------------------------------------------------------------
void StepToDigit(char** ppch)
{
	if (*ppch==0)
		return;

	int len = strlen(*ppch);
	for (int i=0; i<len; i++)
	{
		if (isdigit(**ppch))
			return;
		*ppch += 1;
	}

	*ppch = 0;
}

//------------------------------------------------------------------------
char* UnifyFilepath(char* outPath, const char* szPath)
{
	if (outPath==szPath)
	{
		Error("UnifyFilepath arg error!");
		assert(0);
		return 0;
	}
	if (outPath==0)
		outPath = (char*)malloc(MAX_PATH);
	size_t size = strlen(szPath);
	strcpy_s(outPath, MAX_PATH, szPath);
	int len = strlen(outPath);
	for (int i=0; i<len; i++)
	{
		if (outPath[i]=='\\')
			outPath[i]='/';
	}

	StringVector directories = Split(outPath, "/");
	if (!directories.empty() && directories.back().empty())
	{
		directories.erase(directories.end()-1);
	}

	for (size_t i=0; i<directories.size();)
	{
		if (directories[i]==".")
		{
			directories.erase(directories.begin()+i);
		}
		else if (directories[i]=="..")
		{
			if (i>=1 && directories[i-1]!="..")
			{
				directories.erase(directories.begin() + i);
				directories.erase(directories.begin() + i -1 );
				i--;
			}
			else
			{
				i++;
			}
		}
		else
		{
			i++;
		}
	}
	int pos = 0;
	FB_FOREACH(it, directories)
	{
		strcpy_s(&outPath[pos], MAX_PATH-pos, it->c_str());
		pos += it->size();
		strcpy_s(&outPath[pos], MAX_PATH-pos, "/");
		pos+=1;
	}
	if (pos==0)
	{
		outPath[0]='.';
		outPath[1] = 0;
	}
	else
	{
		outPath[pos-1] = 0; // delete last '/'
	}
	return outPath;
}

std::string ConcatFilepath(const char* a, const char* b)
{
	std::string strConcat;
	if (a && strlen(a))
	{
		size_t len = strlen(a);	
		strConcat = a;
		if (a[len-1]=='\\' || a[len-1]=='/')
		{		
			strConcat += b;
		}
		else
		{
			strConcat += "/";
			strConcat += b;
		}
	}
	else if (b && strlen(b))
	{
		strConcat = b;
	}

	return strConcat;
}

//------------------------------------------------------------------------
char* ToAbsolutePath(char* outChar, const char* a)
{
	if (outChar==a)
	{
		assert(0);
		Error("ToAbsolutePath arg error!");
		return 0;
	}
	if (strlen(a)<=3)
	{
		if (a[1]==':')
		{
			if (!outChar)
			{
				outChar = (char*)malloc(MAX_PATH);
			}
			strcpy(outChar, a);
			return outChar;
		}
	}
	return _fullpath(outChar, a, MAX_PATH);
}

//------------------------------------------------------------------------
StringVector Split(const std::string& str, const std::string& delims /*= "\t\n, "*/, 
		unsigned int maxSplits /*= 0*/, bool preserveDelims /*= false*/)
{
	StringVector ret;
    ret.reserve(maxSplits ? maxSplits+1 : 10);
    unsigned int numSplits = 0;
    // Use STL methods 
    size_t start, pos;
    start = 0;
    do 
    {
        pos = str.find_first_of(delims, start);
        if (pos == start)
        {
            // Do nothing
			ret.push_back(std::string());
			if (pos != std::string::npos)
				start = pos + 1;
        }
        else if (pos == std::string::npos || (maxSplits && numSplits == maxSplits))
        {
            // Copy the rest of the string
            ret.push_back( str.substr(start) );
            break;
        }
        else
        {
            // Copy up to delimiter
            ret.push_back( str.substr(start, pos - start) );

            if(preserveDelims)
            {
                // Sometimes there could be more than one delimiter in a row.
                // Loop until we don't find any more delims
                size_t delimStart = pos, delimPos;
                delimPos = str.find_first_not_of(delims, delimStart);
                if (delimPos == std::string::npos)
                {
                    // Copy the rest of the string
                    ret.push_back( str.substr(delimStart) );
                }
                else
                {
                    ret.push_back( str.substr(delimStart, delimPos - delimStart) );
                }
            }

            start = pos + 1;
        }
        // parse up to next float data
        start = str.find_first_not_of(delims, start);
        ++numSplits;

    } while (pos != std::string::npos);

    return ret;
}

//-----------------------------------------------------------------------
bool StartsWith(const std::string& str, const std::string& pattern, bool lowerCase/* = true*/)
{
	size_t thisLen = str.length();
    size_t patternLen = pattern.length();
    if (thisLen < patternLen || patternLen == 0)
        return false;

    std::string startOfThis = str.substr(0, patternLen);
    if (lowerCase)
        ToLowerCase(startOfThis);

    return (startOfThis == pattern);
}

//-----------------------------------------------------------------------
void ToLowerCase( std::string& str )
{
	std::transform(
            str.begin(),
            str.end(),
            str.begin(),
			tolower);
}

void ToLowerCaseFirst(std::string& str)
{
	std::transform(
		str.begin(), str.begin() + 1, str.begin(), tolower);
}

//-----------------------------------------------------------------------
void ToUpperCase( std::string& str )
{
	std::transform(
            str.begin(),
            str.end(),
            str.begin(),
			toupper);
}

bool IsNumeric(const char* str)
{
	size_t len = strlen(str);
	for (size_t i = 0; i < len; i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

// STRING CONVERTER
//-----------------------------------------------------------------------
std::string StringConverter::toString(float val, unsigned short precision, 
    unsigned short width, char fill, std::ios::fmtflags flags)
{
    std::stringstream stream;
    stream.precision(precision);
    stream.width(width);
    stream.fill(fill);
    if (flags)
        stream.setf(flags);
    stream << val;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(int val, 
    unsigned short width, char fill, std::ios::fmtflags flags)
{
    std::stringstream stream;
	stream.width(width);
    stream.fill(fill);
    if (flags)
        stream.setf(flags);
    stream << val;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(size_t val, 
    unsigned short width, char fill, std::ios::fmtflags flags)
{
    std::stringstream stream;
	stream.width(width);
    stream.fill(fill);
    if (flags)
        stream.setf(flags);
    stream << val;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(unsigned long val, 
    unsigned short width, char fill, std::ios::fmtflags flags)
{
    std::stringstream stream;
	stream.width(width);
    stream.fill(fill);
    if (flags)
        stream.setf(flags);
    stream << val;
    return stream.str();
}

//-----------------------------------------------------------------------
std::string StringConverter::toStringK(unsigned val)
{
	std::stringstream stream;
	if (val > 1000)
	{
		val += 500;
		val /= 1000;
		stream << val << "K";
	}
	else
	{
		stream << val;
	}
	return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(long val, 
    unsigned short width, char fill, std::ios::fmtflags flags)
{
    std::stringstream stream;
	stream.width(width);
    stream.fill(fill);
    if (flags)
        stream.setf(flags);
    stream << val;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Vec2& val)
{
    std::stringstream stream;
	stream << val.x << " " << val.y;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Vec3& val)
{
    std::stringstream stream;
	stream << val.x << " " << val.y << " " << val.z;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Vec4& val)
{
    std::stringstream stream;
	stream << val.x << " " << val.y << " " << val.z << " " << val.w;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Vec4& val, int w, int precision)
{
	std::stringstream stream;
	stream << std::setw(w);
	stream.precision(precision);	
	stream << std::fixed;
	stream << val.x << " " << val.y << " " << val.z << " " << val.w;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Mat33& val)
{
	std::stringstream stream;
    stream << val[0][0] << " " 
        << val[0][1] << " "             
        << val[0][2] << " "             
        << val[1][0] << " "             
        << val[1][1] << " "             
        << val[1][2] << " "             
        << val[2][0] << " "             
        << val[2][1] << " "             
        << val[2][2];
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(bool val, bool yesNo)
{
    if (val)
    {
        if (yesNo)
        {
            return "yes";
        }
        else
        {
            return "true";
        }
    }
    else
        if (yesNo)
        {
            return "no";
        }
        else
        {
            return "false";
        }
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Mat44& val)
{
	std::stringstream stream;
    stream << val[0][0] << " " 
        << val[0][1] << " "             
        << val[0][2] << " "             
        << val[0][3] << " "             
        << val[1][0] << " "             
        << val[1][1] << " "             
        << val[1][2] << " "             
        << val[1][3] << " "             
        << val[2][0] << " "             
        << val[2][1] << " "             
        << val[2][2] << " "             
        << val[2][3] << " "             
        << val[3][0] << " "             
        << val[3][1] << " "             
        << val[3][2] << " "             
        << val[3][3];
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Quat& val)
{
	std::stringstream stream;
    stream  << val.w << " " << val.x << " " << val.y << " " << val.z;
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const Color& val)
{
	std::stringstream stream;
    stream << val.r() << " " << val.g() << " " << val.b() << " " << val.a();
    return stream.str();
}
//-----------------------------------------------------------------------
std::string StringConverter::toString(const StringVector& val)
{
	std::stringstream stream;
    StringVector::const_iterator i, iend, ibegin;
    ibegin = val.begin();
    iend = val.end();
    for (i = ibegin; i != iend; ++i)
    {
        if (i != ibegin)
            stream << " ";

        stream << *i; 
    }
    return stream.str();
}

//-----------------------------------------------------------------------
std::string StringConverter::toString(const RECT& rect)
{
	std::stringstream stream;
	stream << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom;
    return stream.str();
}
//-----------------------------------------------------------------------
float StringConverter::parseReal(const std::string& val, float defaultValue)
{
	// Use istd::stringstream for direct correspondence with toString
	std::stringstream str(val);
	float ret = defaultValue;
	str >> ret;
    return ret;
}
//-----------------------------------------------------------------------
int StringConverter::parseInt(const std::string& val, int defaultValue)
{
	// Use istd::stringstream for direct correspondence with toString
	std::stringstream str(val);
	int ret = defaultValue;
	str >> ret;

    return ret;
}
//-----------------------------------------------------------------------
unsigned int StringConverter::parseUnsignedInt(const std::string& val, unsigned int defaultValue)
{
	// Use istd::stringstream for direct correspondence with toString
	std::stringstream str(val);
	unsigned int ret = defaultValue;
	str >> ret;

	return ret;
}
//-----------------------------------------------------------------------
long StringConverter::parseLong(const std::string& val, long defaultValue)
{
	// Use istd::stringstream for direct correspondence with toString
	std::stringstream str(val);
	long ret = defaultValue;
	str >> ret;

	return ret;
}
//-----------------------------------------------------------------------
unsigned long StringConverter::parseUnsignedLong(const std::string& val, unsigned long defaultValue)
{
	// Use istd::stringstream for direct correspondence with toString
	std::stringstream str(val);
	unsigned long ret = defaultValue;
	str >> ret;

	return ret;
}
unsigned long long StringConverter::parseUnsignedLongLong(const std::string& val, unsigned long long defaultValue)
{
	std::stringstream str(val);
	unsigned long long ret = defaultValue;
	str >> ret;

	return ret;
}

//-----------------------------------------------------------------------
bool StringConverter::parseBool(const std::string& val, bool defaultValue)
{
	if ((StartsWith(val, "true") || StartsWith(val, "yes")
		|| StartsWith(val, "1")))
		return true;
	else if ((StartsWith(val, "false") || StartsWith(val, "no")
		|| StartsWith(val, "0")))
		return false;
	else
		return defaultValue;
}
//-----------------------------------------------------------------------
Vec2 StringConverter::parseVec2(const std::string& val, const Vec2& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() < 2)
    {
        return defaultValue;
    }
    else
    {
        return Vec2(parseReal(vec[0]),parseReal(vec[1]));
    }
}

//-----------------------------------------------------------------------
Vec2I StringConverter::parseVec2I(const std::string& val, const Vec2I defaultValue)
{
	// Split on space
    StringVector vec = Split(val);

    if (vec.size() < 2)
    {
        return defaultValue;
    }
    else
    {
        return Vec2I(parseInt(vec[0]),parseInt(vec[1]));
    }
}

//-----------------------------------------------------------------------
Vec3 StringConverter::parseVec3(const std::string& val, const Vec3& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() < 3)
    {
        return defaultValue;
    }
    else
    {
        return Vec3(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]));
    }
}

Vec3I StringConverter::parseVec3I(const std::string& val, const Vec3& defaultValue)
{
	StringVector vec = Split(val);

    if (vec.size() < 3)
    {
        return defaultValue;
    }
    else
    {
        return Vec3I(parseInt(vec[0]),parseInt(vec[1]),parseInt(vec[2]));
    }
}

//-----------------------------------------------------------------------
Vec4 StringConverter::parseVec4(const std::string& val, const Vec4& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() < 4)
    {
        return defaultValue;
    }
    else
    {
        return Vec4(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]),parseReal(vec[3]));
    }
}
//-----------------------------------------------------------------------
Mat33 StringConverter::parseMat33(const std::string& val, const Mat33& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() < 9)
    {
        return defaultValue;
    }
    else
    {
        return Mat33(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]),
            parseReal(vec[3]),parseReal(vec[4]),parseReal(vec[5]),
            parseReal(vec[6]),parseReal(vec[7]),parseReal(vec[8]));
    }
}
//-----------------------------------------------------------------------
Mat44 StringConverter::parseMat44(const std::string& val, const Mat44& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() < 16)
    {
        return defaultValue;
    }
    else
    {
        return Mat44(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]),
            parseReal(vec[4]),parseReal(vec[5]), parseReal(vec[6]), parseReal(vec[7]),
            parseReal(vec[8]),parseReal(vec[9]), parseReal(vec[10]), parseReal(vec[11]),
            parseReal(vec[12]),parseReal(vec[13]), parseReal(vec[14]), parseReal(vec[15]));
    }
}
//-----------------------------------------------------------------------
Quat StringConverter::parseQuat(const std::string& val, const Quat& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() < 4)
    {
        return defaultValue;
    }
    else
    {
        return Quat(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]));
    }
}
//-----------------------------------------------------------------------
Color StringConverter::parseColor(const std::string& val, const Color& defaultValue)
{
    // Split on space
    StringVector vec = Split(val);

    if (vec.size() == 4)
    {
        return Color(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]));
    }
    else if (vec.size() == 3)
    {
        return Color(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), 1.0f);
    }
    else
    {
        return defaultValue;
    }
}

//-----------------------------------------------------------------------
RECT StringConverter::parseRect(const std::string& val)
{
	StringVector vec = Split(val);
	RECT value = { -123456, -123456, -123456, -123456};
	if (vec.size()==4)
	{
		value.left = parseLong(vec[0]);
		value.top = parseLong(vec[1]);
		value.right = parseLong(vec[2]);
		value.bottom = parseLong(vec[3]);
	}
	else
	{
		assert(0);
	}

	return value;
}

//-----------------------------------------------------------------------
StringVector StringConverter::parseStringVector(const std::string& val)
{
    return Split(val);
}
//-----------------------------------------------------------------------
bool StringConverter::isNumber(const std::string& val)
{
	std::stringstream str(val);
	float tst;
	str >> tst;
	return !str.fail() && str.eof();
}


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
void * HashedString::hash_name(char const * pIdentStr)
{
	// Relatively simple hash of arbitrary text string into a
	// 32-bit identifier Output value is
	// input-valid-deterministic, but no guarantees are made
	// about the uniqueness of the output per-input
	//
	// Input value is treated as lower-case to cut down on false
	// separations cause by human mistypes. Sure, it could be
	// construed as a programming error to mix up your cases, and
	// it cuts down on permutations, but in Real World Usage
	// making this text case-sensitive will likely just lead to
	// Pain and Suffering.
	//
	// This code lossely based upon the adler32 checksum by Mark
	// Adler and published as part of the zlib compression
	// library sources.

	// largest prime smaller than 65536
	unsigned long BASE = 65521L;

	// NMAX is the largest n such that 255n(n+1)/2 +
	// (n+1)(BASE-1) <= 2^32-1
	unsigned long NMAX = 5552;

#define DO1(buf,i)  {s1 += tolower(buf[i]); s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

	if (pIdentStr == NULL)
		return NULL;

	unsigned long s1 = 0;
	unsigned long s2 = 0;

	for (size_t len = strlen(pIdentStr); len > 0;)
	{
		unsigned long k = len < NMAX ? len : NMAX;

		len -= k;

		while (k >= 16)
		{
			DO16(pIdentStr);
			pIdentStr += 16;
			k -= 16;
		}

		if (k != 0) do
		{
			s1 += tolower(*pIdentStr++);
			s2 += s1;
		} while (--k);

		s1 %= BASE;
		s2 %= BASE;
	}

#pragma warning(push)
#pragma warning(disable : 4312)

	return reinterpret_cast<void *>((s2 << 16) | s1);

#pragma warning(pop)
#undef DO1
#undef DO2
#undef DO4
#undef DO8
#undef DO16
}
}