#pragma once
#include "FBCommonHeaders/Types.h"
#include <exception>
#include <string>
#include "FBStringLib/StringLib.h"
namespace fb {
	class UnknownHostException : public std::exception {
		std::string msg;
	public:
		explicit UnknownHostException(const char* _Message)			
		{
			if (_Message) {
				msg = FormatString("UnknownHostException: %s", _Message);
			}
			else {
				msg = "UnknownHostException";
			}
		}

		virtual char const* what() const
		{
			return msg.c_str();
		}
	};

	class SocketException : public std::exception {
	public:
		virtual char const* what() const
		{
			return "Socket exception.";
		}
	};

	class ClosedByInterruptException : public std::exception {
	public:
		virtual char const* what() const
		{
			return "ClosedByInterruptException";
		}
	};

}
