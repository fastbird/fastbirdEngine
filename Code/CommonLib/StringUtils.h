#pragma once
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Color.h>
namespace fastbird
{
	typedef std::vector<std::string> StringVector;
	typedef std::vector<std::wstring> WStringVector;

	char* StripRight(char* s);
	char* StripLeft(char* s);
	std::string StripBoth(const char* s);
	void StripExtension(char* s);
	std::string StripExtension(const char* s);
	const char* FindLastOf(const char* s, char ch);
	std::string GetFileName(const char* s);
	std::string GetFileNameWithoutExtension(const char* s);
	std::string GetDirectoryPath(const char* s);
	const char* StripPath(const char* s);
	const char* GetFileExtension(const char* s);
	bool CheckExtension(const char* filename, const char* extension);
	void StepToDigit(char** ppch);
	// if outPath is zero, you have to free the returned pointer.
	char* UnifyFilepath(char* outPath, const char* szPath);
	bool IsDir(const char* filepath);
	std::string ConcatFilepath(const char* a, const char* b);
	// if outChar is zero, you have to free the returned pointer.
	char* ToAbsolutePath(char* outChar, const char* a);
	StringVector Split(const std::string& str, const std::string& delims = "\t\n, ", 
		unsigned int maxSplits = 0, bool preserveDelims = false);
	bool StartsWith(const std::string& str, const std::string& pattern, bool lowerCase = true);
	void ToLowerCase( std::string& str );
	void ToLowerCaseFirst(std::string& str);
	void ToUpperCase( std::string& str );
	bool IsNumeric(const char* str);

	//-----------------------------------------------------------------
	class StringConverter
    {
    public:

        /** Converts a float to a String. */
        static std::string toString(float val, unsigned short precision = 6, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );

        /** Converts a Radian to a String. */
        /** Converts a Degree to a String. */
        
        /** Converts an int to a String. */
        static std::string toString(int val, unsigned short width = 0, 
            char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );

        /** Converts a size_t to a String. */
        static std::string toString(size_t val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
        /** Converts an unsigned long to a String. */
        static std::string toString(unsigned long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );
		
		static std::string toStringK(unsigned val);

        /** Converts a long to a String. */
        static std::string toString(long val, 
            unsigned short width = 0, char fill = ' ', 
            std::ios::fmtflags flags = std::ios::fmtflags(0) );

		

        /** Converts a boolean to a String. 
        @param yesNo If set to true, result is 'yes' or 'no' instead of 'true' or 'false'
        */
        static std::string toString(bool val, bool yesNo = false);
		/** Converts a Vec2 to a String. 
        @remarks
            Format is "x y" (i.e. 2x float values, space delimited)
        */
        static std::string toString(const Vec2& val);
        /** Converts a Vec3 to a String. 
        @remarks
            Format is "x y z" (i.e. 3x float values, space delimited)
        */
        static std::string toString(const Vec3& val);
		/** Converts a Vec4 to a String. 
        @remarks
            Format is "x y z w" (i.e. 4x float values, space delimited)
        */
        static std::string toString(const Vec4& val);
		static std::string toString(const Vec4& val, int w, int precision);
        /** Converts a Mat33 to a String. 
        @remarks
            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
        */
        static std::string toString(const Mat33& val);
        /** Converts a Mat44 to a String. 
        @remarks
            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc.
        */
        static std::string toString(const Mat44& val);
        /** Converts a Quat to a String. 
        @remarks
            Format is "w x y z" (i.e. 4x float values, space delimited)
        */
        static std::string toString(const Quat& val);
        /** Converts a Color to a String. 
        @remarks
            Format is "r g b a" (i.e. 4x float values, space delimited). 
        */
        static std::string toString(const Color& val);
        /** Converts a StringVector to a string.
        @remarks
            Strings must not contain spaces since space is used as a delimiter in
            the output.
        */
        static std::string toString(const StringVector& val);

		static std::string toString(const RECT& rect);

        /** Converts a std::string to a Real. 
        @returns
            0.0 if the value could not be parsed, otherwise the float version of the String.
        */
        static float parseReal(const std::string& val, float defaultValue = 0);
        
        /** Converts a std::string to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static int parseInt(const std::string& val, int defaultValue = 0);
        /** Converts a std::string to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static unsigned int parseUnsignedInt(const std::string& val, unsigned int defaultValue = 0);
        /** Converts a std::string to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static long parseLong(const std::string& val, long defaultValue = 0);
        /** Converts a std::string to a whole number. 
        @returns
            0.0 if the value could not be parsed, otherwise the numeric version of the String.
        */
        static unsigned long parseUnsignedLong(const std::string& val, unsigned long defaultValue = 0);
		static unsigned long long parseUnsignedLongLong(const std::string& val, unsigned long long defaultValue = 0);
        /** Converts a std::string to a boolean. 
        @remarks
            Returns true if case-insensitive match of the start of the string
			matches "true", "yes" or "1", false otherwise.
        */
        static bool parseBool(const std::string& val, bool defaultValue = 0);
		/** Parses a Vec2 out of a String.
        @remarks
            Format is "x y" ie. 2 float components, space delimited. Failure to parse returns
            Vec2::ZERO.
        */
		static Vec2 parseVec2(const std::string& val, const Vec2& defaultValue = Vec2::ZERO);
		/** Parses a Vec3 out of a String.
        @remarks
            Format is "x y z" ie. 3 float components, space delimited. Failure to parse returns
            Vec3::ZERO.
        */

		static Vec2I parseVec2I(const std::string& val, const Vec2I defaultValue = Vec2I::ZERO);

        static Vec3 parseVec3(const std::string& val, const Vec3& defaultValue = Vec3::ZERO);
        /** Parses a Vec4 out of a String.
        @remarks
            Format is "x y z w" ie. 4 float components, space delimited. Failure to parse returns
            Vec4::ZERO.
        */

		static Vec3I parseVec3I(const std::string& val, const Vec3& defaultValue = Vec3I::ZERO);
        static Vec4 parseVec4(const std::string& val, const Vec4& defaultValue = Vec4::ZERO);
        /** Parses a Mat33 out of a String.
        @remarks
            Format is "00 01 02 10 11 12 20 21 22" where '01' means row 0 column 1 etc.
            Failure to parse returns Mat33::IDENTITY.
        */
        static Mat33 parseMat33(const std::string& val, const Mat33& defaultValue = Mat33::IDENTITY);
        /** Parses a Mat44 out of a String.
        @remarks
            Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33" where 
            '01' means row 0 column 1 etc. Failure to parse returns Mat44::IDENTITY.
        */
        static Mat44 parseMat44(const std::string& val, const Mat44& defaultValue = Mat44::IDENTITY);
        /** Parses a Quat out of a String. 
        @remarks
            Format is "w x y z" (i.e. 4x float values, space delimited). 
            Failure to parse returns Quat::IDENTITY.
        */
        static Quat parseQuat(const std::string& val, const Quat& defaultValue = Quat::IDENTITY);
        /** Parses a Color out of a String. 
        @remarks
            Format is "r g b a" (i.e. 4x float values, space delimited), or "r g b" which implies
            an alpha value of 1.0 (opaque). Failure to parse returns Color::Black.
        */
        static Color parseColor(const std::string& val, const Color& defaultValue = Color::Black);

		static RECT parseRect(const std::string& val); // defulat value is -123456, -123456, -123456

        /** Pareses a StringVector from a string.
        @remarks
            Strings must not contain spaces since space is used as a delimiter in
            the output.
        */
        static StringVector parseStringVector(const std::string& val);

        /** Checks the std::string is a valid number value. */
        static bool isNumber(const std::string& val);
    };


	//-----------------------------------------------------------------
	class HashedString
	{
		// note: mIdent is stored as a void* not an int, so that in
		// the debugger it will show up as hex-values instead of
		// integer values. This is a bit more representative of what
		// we're doing here and makes it easy to allow external code
		// to assign event types as desired.

		void *             mIdent;
		std::string		   mIdentStr;

	public:
		explicit HashedString(char const * const pIdentString)
			: mIdent(hash_name(pIdentString))
			, mIdentStr(pIdentString)
		{
		}

		unsigned long getHashValue(void) const
		{
			return reinterpret_cast<unsigned long>(mIdent);
		}

		const std::string & getStr() const
		{
			return mIdentStr;
		}

		static void* hash_name(char const *  pIdentStr);

		bool operator< (HashedString const & o) const
		{
			bool r = (getHashValue() < o.getHashValue());
			return r;
		}

		bool operator== (HashedString const & o) const
		{
			bool r = (getHashValue() == o.getHashValue());
			return r;
		}
	};
}