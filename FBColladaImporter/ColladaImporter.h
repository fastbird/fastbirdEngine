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
#include "FBCommonHeaders/platform.h"
#include "FBCommonHeaders/IteratorWrapper.h"
#include "FBColladaImporter/FBColladaData.h"
namespace COLLADAFW
{
	class Mesh;
	class MeshVertexData;
	class Node;
	class FloatOrDoubleArray;
}
namespace fb
{
	namespace collada{
		struct Mesh;
		typedef std::shared_ptr<Mesh> MeshPtr;
		struct MeshGroup;
		typedef std::shared_ptr<MeshGroup> MeshGroupPtr;
	}
	typedef std::map<std::string, collada::MeshPtr> ColladaMeshObjects;
	struct ImportOptions;		
	FB_DECLARE_SMART_PTR(ColladaImporter);		
	class FB_DLL_COLLADA ColladaImporter
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(ColladaImporter);

	protected:
		ColladaImporter();
		~ColladaImporter();

	public:

		static ColladaImporterPtr Create();
		// only call once before the process terminates
		static void CleanUP();

		struct ImportOptions{
			bool mSwapYZ;
			bool mOppositeCull;
			bool mUseIndexBuffer;
			bool mMergeMaterialGroups;
			bool mUseMeshGroup;

			ImportOptions()
				: mSwapYZ(false)
				, mOppositeCull(true)
				, mUseIndexBuffer(false)
				, mMergeMaterialGroups(false)
				, mUseMeshGroup(false)
			{
			}
		};
			
		bool ImportCollada(const char* filepath);
		bool ImportCollada(const char* filepath, const ImportOptions& options);			
		collada::MeshPtr GetMeshObject() const;
		collada::MeshGroupPtr GetMeshGroup() const;
		IteratorWrapper<ColladaMeshObjects> GetMeshIterator();
		unsigned GetNumMeshes() const;
	};
}