#pragma once
namespace fb {

	template<class T>
	class Buffer {
		std::vector<T> mData;
		unsigned mPosition;

	public:
		Buffer()
			:mPosition(0)
		{
		}

		unsigned Position() const {
			return mPosition;
		}

		unsigned Limit() const {
			return mData.size();
		}

		unsigned Size() const {
			return mData.size();
		}

		T& operator[](unsigned index) {
			if (index >= mData.size()) {
				throw std::exception("Out of index");
			}
			return mData[index];
		}

		T Get() {
			if (mPosition >= mData.size()) {
				return T();
			}

			return mData[mPosition++];
		}

		Buffer<T>& put(const T& t) {
			mData.insert(mData.begin() + mPosition, t);
			++mPosition;
			return *this;
		}
	};

	typedef Buffer<int> IntBuffer;
	typedef Buffer<short> ShortBuffer;
	typedef Buffer<Real> RealBuffer;

}
