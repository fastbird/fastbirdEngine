#pragma once

#include <map>

namespace fastbird
{
	//------------------------------------------------------------------------
	template<class _Kty, class _Ty>
	class VectorMap
	{
	public:

		typedef std::pair< _Kty, _Ty > _Val_type;
		typedef std::pair<const _Kty, _Ty> value_type;
		typedef _Kty key_type;		
		typedef std::vector< _Val_type > VectorMapType;
		typedef typename VectorMapType::iterator iterator;
		typedef typename VectorMapType::const_iterator const_iterator;
		typedef typename VectorMapType::reverse_iterator reverse_iterator;
		typedef typename VectorMapType::const_reverse_iterator const_reverse_iterator;
		typedef typename _Val_type::first_type PairFirstType;

		//--------------------------------------------------------------------
		class Comparison
		{
		public:
			bool operator()(const value_type& lhs, const value_type& rhs) const
			{
				return return lhs.first < rhs.first;
			}

			bool operator()(const value_type& lhs, const PairFirstType& key) const
			{
				return lhs.first < key;
			}

			bool operator()(const PairFirstType& key, const value_type& rhs) const
			{
				return key < rhs.first;
			}
		};
		
		//--------------------------------------------------------------------
		VectorMap()
		{

		}

		//--------------------------------------------------------------------
		iterator Insert(const _Val_type& data)
		{
			iterator i = std::lower_bound(
				mVector.begin(), mVector.end(), data.first, Comparison());
			if(i != mVector.end() && i->first == data.first)
			{
				i->second = data.second;		
			}	
			else
			{
				i = mVector.insert(i, data);
			}
			return i;
		}

		//--------------------------------------------------------------------
		const_iterator Insert(const _Val_type& data) const 
		{
			const_iterator i = std::lower_bound(
				mVector.begin(), mVector.end(), data.first, Comparison());
			if(i != mVector.end() && i->first == data.first)
			{
				i->second = data.second;		
			}	
			else
			{
				i = mVector.insert(i, data);
			}
			return i;
		}

		//--------------------------------------------------------------------
		_Ty& operator [](const key_type& key )
		{
			iterator i =std::lower_bound(
				mVector.begin(), mVector.end(), key, Comparison());
			if(i != mVector.end() && i->first == key)
			{
				return i->second;
			}	
			else
			{
				_Ty temp;
				_Val_type data( key, temp);
				i = mVector.insert(i, data);
				return i->second;
			}		
		}

		//--------------------------------------------------------------------
		const _Ty& operator [](const key_type& key ) const
		{
			const_iterator begin = mVector.begin(), end = mVector.end();
			const_iterator i = std::lower_bound(begin, end, key, Comparison());
			if(i != end && i->first == key)
			{
				return i->second;
			}		
			return i->second;
		}

		//--------------------------------------------------------------------
		iterator Find( const key_type& Key )
		{
			iterator i = std::lower_bound(
				mVector.begin(), mVector.end(), Key, Comparison());
			if(i != mVector.end() && i->first == Key)
			{
				return i;
			}
			else
			{
				return mVector.end();
			}
		}

		//--------------------------------------------------------------------
		const_iterator Find( const key_type& Key ) const
		{
			const_iterator i =
				std::lower_bound(mVector.begin(), mVector.end(), Key, Comparison());
			if(i != mVector.end() && i->first == Key)
			{
				return i;
			}
			else
			{
				return mVector.end();
			}
		}

		//--------------------------------------------------------------------
		bool operator==( const VectorMapType& vec )const
		{
			const VectorMapType& other = static_cast<const VectorMapType&>( vec );
			return mVector == other.mVector;
		}

		//--------------------------------------------------------------------
		inline iterator begin() 
		{
			return mVector.begin();
		}

		inline const_iterator begin() const
		{
			const_iterator it = mVector.begin();
			return it;
		}

		inline iterator end() 
		{
			return mVector.end();
		}	

		inline const_iterator end() const
		{
			const_iterator it = mVector.end();
			return it;
		}

		//--------------------------------------------------------------------
		inline reverse_iterator rbegin()
		{
			return mVector.rbegin();
		}

		inline const_reverse_iterator rbegin() const
		{
			return mVector.rbegin();
		}

		inline reverse_iterator rend()
		{
			return mVector.rend();
		}

		inline const_reverse_iterator rend() const
		{
			return mVector.rend();
		}

		//--------------------------------------------------------------------
		const size_t size() const
		{
			return mVector.size();
		}

		//--------------------------------------------------------------------
		inline void clear() 
		{
			return mVector.clear();
		}	

		//--------------------------------------------------------------------
		iterator erase(iterator it)
		{
			return mVector.erase(it);
		}

		//--------------------------------------------------------------------
		inline bool empty() const
		{
			return mVector.empty();
		}


	private:

		std::vector< std::pair< _Kty,_Ty > > mVector;
	};
}