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

#pragma once

#include <vector>
#include <algorithm>

namespace fb
{
	template<class _Kty, class _Ty>
	class VectorMapLess
	{
	public:
		typedef std::pair<const _Kty, _Ty> value_type;
		typedef std::pair< _Kty, _Ty > _Val_type;
		typedef typename _Val_type::first_type PairFirstType;

		bool operator()(const value_type& lhs, const value_type& rhs) const
		{
			return lhs.first < rhs.first;
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
	//------------------------------------------------------------------------
	template<class _Kty, class _Ty, class _Pr = VectorMapLess<_Kty, _Ty> >
	class VectorMap
	{
	public:

		typedef VectorMap<_Kty, _Ty, _Pr> ThisType;
		typedef std::pair< _Kty, _Ty > _Val_type;
		typedef std::pair<const _Kty, _Ty> value_type;
		typedef _Kty key_type;		
		typedef _Pr key_compare;
		typedef std::vector< _Val_type > VectorMapType;
		typedef typename VectorMapType::iterator iterator;
		typedef typename VectorMapType::const_iterator const_iterator;
		typedef typename VectorMapType::reverse_iterator reverse_iterator;
		typedef typename VectorMapType::const_reverse_iterator const_reverse_iterator;
		typedef typename _Val_type::first_type PairFirstType;
		
		
		//--------------------------------------------------------------------
		VectorMap()
		{

		}

		virtual ~VectorMap() {

		}

		//--------------------------------------------------------------------
		iterator Insert(const _Val_type& data)
		{
			iterator i = std::lower_bound(
				mVector.begin(), mVector.end(), data.first, key_compare());
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
				mVector.begin(), mVector.end(), key, key_compare());
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
			const_iterator i = std::lower_bound(begin, end, key, key_compare());
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
				mVector.begin(), mVector.end(), Key, key_compare());
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
				std::lower_bound(mVector.begin(), mVector.end(), Key, key_compare());
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
		bool operator==( const ThisType& other )const
		{
			return mVector == other.mVector;
		}

		bool operator!=(const ThisType& other) const
		{
			return mVector != other.mVector;
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

		//--------------------------------------------------------------------
		void swap(VectorMap<_Kty, _Ty>& other){
			mVector.swap(other.mVector);
		}


	private:

		std::vector< std::pair< _Kty,_Ty > > mVector;
	};
}