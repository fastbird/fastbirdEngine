#pragma once
namespace fb{
	template < class T,
		typename std::enable_if<std::is_pod<T>::value, void>::type* = nullptr >
	void write(std::ostream& out, const std::vector<T>& data)
	{
		auto size = data.size();
		out.write((char*)&size, sizeof(size));
		for (const auto& d : data){
			out.write((char*)&d, sizeof(d));
		}
	}

	template <class T>
	void write(std::ostream& out, const std::vector<T>& data)
	{
		auto size = data.size();
		out.write((char*)&size, sizeof(size));
		for (const auto& d : data){
			d.write(out);
		}
	}

	template < class T,
		typename std::enable_if<std::is_pod<T>::value, void>::type* = nullptr >
	void read(std::istream& out, std::vector<T>& data)
	{
		size_t size;
		out.read((char*)&size, sizeof(size));
		data.clear();
		data.reserve(size);
		for (sizt_t i = 0; i < size; ++i){
			data.push_back(T());
			auto& dest = data.back();
			out.read((char*)&dest, sizeof(T));
		}		
	}

	template <class T>
	void read(std::istream& out, std::vector<T>& data)
	{
		size_t size;
		out.read((char*)&size, sizeof(size));
		data.clear();
		data.reserve(size);
		for (size_t i = 0; i < size; ++i){
			data.push_back(T());
			auto& dest = data.back();
			dest.read(out);
		}
	}
}