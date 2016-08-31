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

#include "stdafx.h"
#include "binary_mesh.h"
#include "FBSceneObjectFactory/MeshGroup.h"
#include "TinyXmlLib/tinyxml2.h"
#include "FBFileSystem/FileSystem.h"
#include "FBColladaImporter/ColladaImporter.h"
#include "FBStringLib/StringConverter.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/level.hpp>
#include <boost/scope_exit.hpp>

namespace fb {
	enum : unsigned {
		// version : 20160800 -- 2016 year / 08 month / 00th
		fbmesh_version = 20160800
	};

	static const char mesh_mark[2] = { (char)0xfb, (char)0x1 };
	static const char mesh_group_mark[2] = { (char)0xfb, (char)0x2 };
	static const char meshes_mark[2] = { (char)0xfb, (char)0x3 };

	struct mesh_file_header {
		char mark[2];
		unsigned version = fbmesh_version;
		unsigned numDescs = 0;

		bool is_mesh() const {
			return memcmp(mark, mesh_mark, 2) == 0;
		}
		bool is_meshes() const {
			return memcmp(mark, meshes_mark, 2) == 0;
		}
		bool is_mesh_group() const {
			return memcmp(mark, mesh_group_mark, 2) == 0;
		}
	};
	// format
	// mark: (0xfb, [0x1 -- meshe, 0x2 -- mesh group])
	// version	
	// mesh1	
	//		mesh group
	// mesh2	
	//		mesh group	

	// number of descs -- header start
	// mesh_desc + pos
	// mesh_desc + pos
	// data end pos


	dae_descs parse_descs(const std::string& daedesc_path) {
		dae_descs descs;
		if (!FileSystem::Exists(daedesc_path.c_str())) {
			return descs;
		}

		tinyxml2::XMLDocument doc;
		auto err = doc.LoadFile(daedesc_path.c_str());
		if (err) {
			std::cerr << "Cannot parse daedesc.\n";
			std::cerr << doc.GetErrorStr1() << std::endl;
			return descs;
		}

		auto root = doc.RootElement();
		if (!root) {
			std::cerr << "No root element.\n";
			return descs;
		}
		auto sz = root->Attribute("fractured");
		if (sz) {
			descs.fractured = StringConverter::ParseBool(sz);
		}

		sz = root->Attribute("mesh_group");
		if (sz) {
			descs.mesh_group = StringConverter::ParseBool(sz);
		}

		auto elem = root->FirstChildElement("daedesc");
		while (elem) {
			descs.descs.push_back(MeshImportDesc());
			auto& desc = descs.descs.back();
			auto attrbElem = elem->FirstChildElement("yzSwap");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.yzSwap = StringConverter::ParseBool(sz);
				}
			}
			attrbElem = elem->FirstChildElement("oppositeCull");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.oppositeCull = StringConverter::ParseBool(sz);
				}
			}
			attrbElem = elem->FirstChildElement("useIndexBuffer");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.useIndexBuffer = StringConverter::ParseBool(sz);
				}
			}
			attrbElem = elem->FirstChildElement("mergeMaterialGroups");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.mergeMaterialGroups = StringConverter::ParseBool(sz);
				}
			}
			attrbElem = elem->FirstChildElement("keepMeshData");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.keepMeshData = StringConverter::ParseBool(sz);
				}
			}
			attrbElem = elem->FirstChildElement("generateTangent");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.generateTangent = StringConverter::ParseBool(sz);
				}
			}
			attrbElem = elem->FirstChildElement("ignore_cache");
			if (attrbElem) {
				auto sz = attrbElem->GetText();
				if (sz) {
					desc.ignore_cache = StringConverter::ParseBool(sz);
				}
			}

			elem = elem->NextSiblingElement();
		}

		return descs;
	}	

	template <class Archive>
	void RegisterType(Archive& ar) {		
		/*ar.register_type<mesh_headers>();
		ar.register_type<mesh_header>();
		ar.register_type<collada::Mesh>();
		ar.register_type<collada::MeshGroup>();		
		ar.register_type<std::map<int, collada::MaterialGroup>>();
		ar.register_type<collada::MaterialGroup>();
		ar.register_type<collada::IndexBuffer>();
		ar.register_type<collada::ModelTriangle>();
		ar.register_type<AnimationData>();*/
	}

	FB_DLL_SCENEOBJECTFACTORY
		bool save_meshes(const char* dae_filepath) {
		if (!ValidCString(dae_filepath)) {
			std::cerr << "Invalid arg.\n";
			return false;
		}

		if (!FileSystem::Exists(dae_filepath)) {
			std::cerr << "File not exists.\n";
			return false;
		}

		std::string daedesc_path = dae_filepath;
		daedesc_path = FileSystem::ReplaceExtension(daedesc_path.c_str(), "daedesc");
		auto descs = parse_descs(daedesc_path);

		if (descs.descs.empty()) {
			descs.descs.push_back(MeshImportDesc());
		}
		mesh_file_header file_header;
		memcpy(file_header.mark, mesh_mark, 2);
		auto meshfile_path = FileSystem::ReplaceExtension(dae_filepath, "fbmesh");
		if (descs.mesh_group) {
			meshfile_path += "_group";
			memcpy(file_header.mark, mesh_group_mark, 2);
		}
		else if (descs.fractured) {
			meshfile_path += "es";
			memcpy(file_header.mark, meshes_mark, 2);
		}

		auto backuped_file_path = FileSystem::BackupFile(meshfile_path.c_str(), 1);
		bool created = false;
		BOOST_SCOPE_EXIT(&backuped_file_path, &meshfile_path, &created) {
			if (!backuped_file_path.empty()) {
				if (!created) {
					FileSystem::CopyFile(backuped_file_path.c_str(), meshfile_path.c_str(), true, true);
				}
				FileSystem::Remove(backuped_file_path.c_str());
			}
		}BOOST_SCOPE_EXIT_END;
		mesh_headers headers;
		meshes_headers fractured_headers;
		{
			std::ofstream stream(meshfile_path, std::ios::binary);
			boost::archive::binary_oarchive ar(stream);			
			RegisterType(ar);
			file_header.numDescs = descs.descs.size();
			ar & file_header;
			for (auto& desc : descs.descs) {
				auto pColladaImporter = ColladaImporter::Create();
				ColladaImporter::ImportOptions option;
				option.mMergeMaterialGroups = desc.mergeMaterialGroups;
				option.mOppositeCull = desc.oppositeCull;
				option.mSwapYZ = desc.yzSwap;
				option.mUseIndexBuffer = desc.useIndexBuffer;
				option.mUseMeshGroup = descs.mesh_group;
				if (!pColladaImporter->ImportCollada(dae_filepath, option)) {
					std::cerr << "Failed to import " << dae_filepath << "\n";
					return false;
				}
				if (descs.mesh_group) {
					auto meshGroup = pColladaImporter->GetMeshGroup();
					if (meshGroup) {
						mesh_header header;
						header.desc = desc;
						ar & header;
						ar & (*meshGroup);
					}
					else {
						std::cerr << "Cannot load mesh.\n";
						return false;
					}
				}
				else if (descs.fractured) {
					meshes_header h;					
					h.numMeshes = pColladaImporter->GetNumMeshes();
					h.desc = desc;
					ar & h;
					auto meshIt = pColladaImporter->GetMeshIterator();
					if (!meshIt.HasMoreElement()) {
						std::cerr << "Failed to load fracture meshes.\n";
						return false;
					}
					while (meshIt.HasMoreElement()) {
						auto it = meshIt.GetNext();
						ar & it.first;
						ar & (*it.second);
					}
				}
				else {
					auto meshData = pColladaImporter->GetMeshObject();
					if (meshData) {
						mesh_header h;						
						h.desc = desc;						
						ar & h;
						ar & (*meshData);
					}
					else {
						std::cerr << "Cannot load mesh.\n";
						return false;
					}
				}
			}

			created = true;
		}

		// input test
		if (descs.mesh_group) {
			auto mg = load_mesh_group(meshfile_path.c_str(), descs.descs[0]);
			if (!mg) {
				std::cerr << "loading test failed for " << meshfile_path << std::endl;
			}
		}


		return true;
	}

	FB_DLL_SCENEOBJECTFACTORY
	collada::MeshPtr load_mesh(const char* mesh_path, const MeshImportDesc& desc) {
		if (!FileSystem::Exists(mesh_path))
		{
			std::cerr << "File not exists. " << mesh_path << "\n";
			return false;
		}
		std::ifstream stream(mesh_path, std::ios::binary);
		if (!stream) {
			std::cerr << "Cannot open the file. " << mesh_path << "\n";
			return false;
		}
		return load_mesh(stream, mesh_path, desc);		
	}

	FB_DLL_SCENEOBJECTFACTORY
		collada::MeshPtr load_mesh(std::istream& stream, const char* mesh_path, const MeshImportDesc& desc)
	{
		boost::archive::binary_iarchive ar(stream);		
		RegisterType(ar);
		mesh_file_header file_header;
		ar & file_header;
		if (!file_header.is_mesh()) {
			std::cerr << "This file is not a mesh file. " << mesh_path << "\n";
			return false;
		}
		for (unsigned i = 0; i < file_header.numDescs; ++i) {
			mesh_header h;
			ar & h;
			collada::MeshPtr pMesh = std::make_shared<collada::Mesh>();
			ar & (*pMesh);
			if (h.desc == desc)
				return pMesh;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Cannot load mesh(%s) with desc(%s)", mesh_path, desc.ToString().c_str()).c_str());
		
		return nullptr;
	}

	FB_DLL_SCENEOBJECTFACTORY
	std::vector<collada::MeshPtr> load_meshes(const char* mesh_path, const  MeshImportDesc& desc) {
		if (!FileSystem::Exists(mesh_path))
		{
			std::cerr << "File not exists. " << mesh_path << "\n";
			return {};
		}
		std::ifstream stream(mesh_path, std::ios::binary);
		if (!stream) {
			std::cerr << "Cannot open the file. " << mesh_path << "\n";
			return{};
		}
		return load_meshes(stream, mesh_path, desc);
	}	

	FB_DLL_SCENEOBJECTFACTORY
		std::vector<collada::MeshPtr> load_meshes(std::istream& stream, const char* mesh_path, const MeshImportDesc& desc)
	{
		boost::archive::binary_iarchive ar(stream);		
		RegisterType(ar);
		mesh_file_header file_header;
		ar & file_header;
		if (!file_header.is_meshes()) {
			std::cerr << "This file is not a mesh fractured file. " << mesh_path << "\n";
			return{};
		}

		for (unsigned i = 0; i < file_header.numDescs; ++i) {
			meshes_header h;
			ar & h;
			std::vector<collada::MeshPtr> meshes;
			meshes.reserve(h.numMeshes);
			for (unsigned i = 0; i < h.numMeshes; ++i) {
				std::string meshname;
				ar & meshname;
				collada::MeshPtr pMesh = std::make_shared<collada::Mesh>();
				ar & (*pMesh);
				meshes.push_back(pMesh);
			}
			if (h.desc == desc){
				return meshes;
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot load meshes(%s) with desc(%s)",
			mesh_path, desc.ToString().c_str()).c_str());
		return{};
	}

	FB_DLL_SCENEOBJECTFACTORY
	collada::MeshGroupPtr load_mesh_group(const char* mesh_path, const MeshImportDesc& desc) {
		if (!FileSystem::Exists(mesh_path))
		{
			std::cerr << "File not exists. " << mesh_path << "\n";
			return false;
		}
		std::ifstream stream(mesh_path, std::ios::binary);
		if (!stream) {
			std::cerr << "Cannot open the file. " << mesh_path << "\n";
			return false;
		}
		return load_mesh_group(stream, mesh_path, desc);
		
	}

	FB_DLL_SCENEOBJECTFACTORY
		collada::MeshGroupPtr load_mesh_group(std::istream& stream, const char* mesh_path, const MeshImportDesc& desc)
	{

		boost::archive::binary_iarchive ar(stream);		
		RegisterType(ar);		
		mesh_file_header file_header;
		ar & file_header;
		if (!file_header.is_mesh_group()) {
			std::cerr << "This file is not a mesh group file. " << mesh_path << "\n";
			return false;
		}
		
		for (unsigned i = 0; i < file_header.numDescs; ++i) {
			mesh_header h;
			ar & h;
			collada::MeshGroupPtr pMeshGroup = std::make_shared<collada::MeshGroup>();
			ar & (*pMeshGroup);
			if (h.desc == desc){
				return pMeshGroup;			
			}
		}

		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Cannot load mesh(%s) with desc(%s)", mesh_path, desc.ToString().c_str()).c_str());

		return nullptr;
	}
}

BOOST_CLASS_IMPLEMENTATION(fb::mesh_file_header, boost::serialization::primitive_type);
