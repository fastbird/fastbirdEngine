#include <CommonLib/StdAfx.h>
#include "INIReader.h"
#include "StringUtils.h"

/* Nonzero to allow multi-line value parsing, in the style of Python's
   ConfigParser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#define INI_ALLOW_MULTILINE 1

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#define INI_ALLOW_BOM 1
#define INI_USE_STACK 1
#define INI_MAX_LINE 200
#define MAX_SECTION 50
#define MAX_NAME 50

using namespace fastbird;

//----------------------------------------------------------------------------
INIReader::INIReader(const char* filename)
{
    mError = ini_parse(filename, ValueHandler, this);
}

void INIReader::FinishSmartPtr(){
	FB_DELETE(this);
}

//----------------------------------------------------------------------------
int INIReader::GetError()
{
    return mError;
}

//----------------------------------------------------------------------------
std::string INIReader::Get(const char* section, const char* name, const char* default_value)
{
    std::string key = MakeKey(section, name);
    return mValues.count(key) ? mValues[key] : default_value;
}

//----------------------------------------------------------------------------
long INIReader::GetInteger(const char* section, const char* name, long default_value)
{
    std::string valstr = Get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : default_value;
}

//----------------------------------------------------------------------------
bool INIReader::GetBoolean(const char* section, const char* name, bool default_value)
{
    std::string valstr = Get(section, name, "");
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
    if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        return true;
    else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        return false;
    else
        return default_value;
}

//----------------------------------------------------------------------------
std::string INIReader::MakeKey(const char* section, const char* name)
{
    std::string key = std::string(section) + "." + name;
    // Convert to lower case to make section/name lookups case-insensitive
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    return key;
}

//----------------------------------------------------------------------------
int INIReader::ValueHandler(void* user, const char* section, const char* name,
                            const char* value)
{
    INIReader* reader = (INIReader*)user;
    std::string key = MakeKey(section, name);
    if (reader->mValues[key].size() > 0)
        reader->mValues[key] += "\n";
    reader->mValues[key] += value;
    return 1;
}

//----------------------------------------------------------------------------
namespace fastbird
{
	//------------------------------------------------------------------------
	/* Return pointer to first char c or ';' comment in given string, or pointer to
	null at end of string if neither found. ';' must be prefixed by a whitespace
	character to register as a comment. */
	static char* find_char_or_comment(const char* s, char c)
	{
		int was_whitespace = 0;
		while (*s && *s != c && !(was_whitespace && *s == ';')) {
			was_whitespace = isspace((unsigned char)(*s));
			s++;
		}
		return (char*)s;
	}

	//------------------------------------------------------------------------
	/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
	static char* strncpy0(char* dest, size_t bufferSize, const char* src, size_t size)
	{
		strncpy_s(dest, bufferSize, src, size);
		dest[size - 1] = '\0';
		return dest;
	}

	//------------------------------------------------------------------------
	/* See documentation in header file. */
	int ini_parse_file(FILE* file,
					   int (*handler)(void*, const char*, const char*,
									  const char*),
					   void* user)
	{
		/* Uses a fair bit of stack (use heap instead if you need to) */
	#if INI_USE_STACK
		char line[INI_MAX_LINE];
	#else
		char* line;
	#endif
		char section[MAX_SECTION] = "";
		char prev_name[MAX_NAME] = "";

		char* start;
		char* end;
		char* name;
		char* value;
		int lineno = 0;
		int error = 0;

	#if !INI_USE_STACK
		line = (char*)malloc(INI_MAX_LINE);
		if (!line) {
			return -2;
		}
	#endif

		/* Scan through file line by line */
		while (fgets(line, INI_MAX_LINE, file) != NULL) {
			lineno++;

			start = line;
	#if INI_ALLOW_BOM
			if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
							   (unsigned char)start[1] == 0xBB &&
							   (unsigned char)start[2] == 0xBF) {
				start += 3;
			}
	#endif
			start = StripLeft(StripRight(start));

			if (*start == ';' || *start == '#') {
				/* Per Python ConfigParser, allow '#' comments at start of line */
			}
	#if INI_ALLOW_MULTILINE
			else if (*prev_name && *start && start > line) {
				/* Non-black line with leading whitespace, treat as continuation
				   of previous name's value (as per Python ConfigParser). */
				if (!handler(user, section, prev_name, start) && !error)
					error = lineno;
			}
	#endif
			else if (*start == '[') {
				/* A "[section]" line */
				end = find_char_or_comment(start + 1, ']');
				if (*end == ']') {
					*end = '\0';
					strncpy0(section, MAX_SECTION, start + 1, sizeof(section));
					*prev_name = '\0';
				}
				else if (!error) {
					/* No ']' found on section line */
					error = lineno;
				}
			}
			else if (*start && *start != ';') {
				/* Not a comment, must be a name[=:]value pair */
				end = find_char_or_comment(start, '=');
				if (*end != '=') {
					end = find_char_or_comment(start, ':');
				}
				if (*end == '=' || *end == ':') {
					*end = '\0';
					name = StripRight(start);
					value = StripLeft(end + 1);
					end = find_char_or_comment(value, '\0');
					if (*end == ';')
						*end = '\0';
					StripRight(value);

					/* Valid name[=:]value pair found, call handler */
					strncpy0(prev_name, MAX_NAME, name, sizeof(prev_name));
					if (!handler(user, section, name, value) && !error)
						error = lineno;
				}
				else if (!error) {
					/* No '=' or ':' found on name[=:]value line */
					error = lineno;
				}
			}
		}

	#if !INI_USE_STACK
		free(line);
	#endif

		return error;
	}

	//------------------------------------------------------------------------
	/* See documentation in header file. */
	int ini_parse(const char* filename,
				  int (*handler)(void*, const char*, const char*, const char*),
				  void* user)
	{
		FILE* file = 0;
		int error;

		error = fopen_s(&file, filename, "r");
		if (error!=0)
			return -1;
		error = ini_parse_file(file, handler, user);
		if (file)
			fclose(file);

		return error;
	}
}