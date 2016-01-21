#include "Serialization.h"
namespace fb{
	void write(std::ostream& stream, const std::string& str){
		size_t size = str.size();
		stream.write((char*)&size, sizeof(size));
		if (size > 0){
			stream.write(&str[0], size);
		}
	}
	void read(std::istream& stream, std::string& str){
		size_t size;
		stream.read((char*)&size, sizeof(size));
		if (size > 0){
			str.assign((size_t)size, (char)0);
			stream.read(&str[0], size);
		}
	}
}