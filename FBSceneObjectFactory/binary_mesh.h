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
#include "MeshImportDesc.h"
#include <vector>
namespace fb {
	namespace collada {
		struct Mesh;
		typedef std::shared_ptr<Mesh> MeshPtr;
		struct MeshGroup;
		typedef std::shared_ptr<MeshGroup> MeshGroupPtr;
	}
}
namespace fb {
	FB_DECLARE_SMART_PTR(MeshObject);

	struct dae_descs {
		bool mesh_group;
		bool fractured;
		std::vector<MeshImportDesc> descs;

		dae_descs()
			: mesh_group(false)
			, fractured(false)
		{

		}
	};

	struct mesh_header {
		MeshImportDesc desc;		

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & desc;
		}
	};

	struct meshes_header {
		unsigned numMeshes;
		MeshImportDesc desc;
		unsigned start_pos;

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & numMeshes & desc & start_pos;
		}
	};
	
	using mesh_headers = std::vector<mesh_header>;	
	using meshes_headers = std::vector<meshes_header>;

	FB_DLL_SCENEOBJECTFACTORY
		bool save_meshes(const char* dae_filepath);

	FB_DLL_SCENEOBJECTFACTORY
		collada::MeshPtr load_mesh(const char* mesh_path, const MeshImportDesc& desc);		
	FB_DLL_SCENEOBJECTFACTORY
		collada::MeshPtr load_mesh(std::istream & streambuf, const char* mesh_path, const MeshImportDesc& desc);

	FB_DLL_SCENEOBJECTFACTORY
		std::vector<collada::MeshPtr> load_meshes(const char* mesh_path, const MeshImportDesc& desc);	
	FB_DLL_SCENEOBJECTFACTORY
		std::vector<collada::MeshPtr> load_meshes(std::istream& stream, const char* mesh_path, const MeshImportDesc& desc);

	FB_DLL_SCENEOBJECTFACTORY
		collada::MeshGroupPtr load_mesh_group(const char* mesh_path, const MeshImportDesc& desc);	
	FB_DLL_SCENEOBJECTFACTORY
		collada::MeshGroupPtr load_mesh_group(std::istream& streambuf, const char* mesh_path, const MeshImportDesc& desc);
}