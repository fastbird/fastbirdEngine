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
#include <bitset>
#include <boost/serialization/bitset.hpp>
namespace fb {
	struct MeshImportDesc
	{
		bool yzSwap;
		bool oppositeCull;
		bool useIndexBuffer;
		bool mergeMaterialGroups;
		bool keepMeshData;
		bool generateTangent;
		bool ignore_cache;

		MeshImportDesc()
			: yzSwap(false), oppositeCull(true),
			useIndexBuffer(true), mergeMaterialGroups(false),
			keepMeshData(false), generateTangent(true), ignore_cache(false)
		{
		}

		bool operator==(const MeshImportDesc& other) {
			return !(yzSwap != other.yzSwap || oppositeCull != other.oppositeCull ||
				useIndexBuffer != other.useIndexBuffer || mergeMaterialGroups != other.mergeMaterialGroups ||
				keepMeshData != other.keepMeshData || generateTangent != other.generateTangent || ignore_cache != other.ignore_cache);
		}

		std::string ToString() const {
			return FormatString("yzSwap: %d, oppositeCull: %d, useIndexBuffer: %d, mergeMaterialGroups: %d, keepMeshData: %d, generateTangent: %d, ignore_cache: %d",
				yzSwap, oppositeCull, useIndexBuffer, mergeMaterialGroups, keepMeshData, generateTangent, ignore_cache);
		}

	private:
		friend class boost::serialization::access;
		// When the class Archive corresponds to an output archive, the
		// & operator is defined similar to <<.  Likewise, when the class Archive
		// is a type of input archive the & operator is defined similar to >>.
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const
		{
			std::bitset<7> bs;
			bs.set(0, yzSwap);
			bs.set(1, oppositeCull);
			bs.set(2, useIndexBuffer);
			bs.set(3, mergeMaterialGroups);
			bs.set(4, keepMeshData);
			bs.set(5, generateTangent);
			bs.set(6, ignore_cache);
			ar & bs;
		}
		template<class Archive>
		void load(Archive & ar, const unsigned int version){
			std::bitset<7> data;
			ar & data;			
			yzSwap = data[0];
			oppositeCull = data[1];
			useIndexBuffer = data[2];
			mergeMaterialGroups = data[3];
			keepMeshData = data[4];
			generateTangent = data[5];
			ignore_cache = data[6];
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER()


	};

	void write(std::ostream& stream, const MeshImportDesc& data);
	void read(std::istream& stream, MeshImportDesc& data);
}