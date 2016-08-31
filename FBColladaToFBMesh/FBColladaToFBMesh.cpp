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

// FBColladaToFBMesh.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FBSceneObjectFactory/binary_mesh.h"
#include <boost/program_options.hpp>
#include <regex>
#include <set>
using namespace fb;

std::set<std::string> ignores;

bool ignore(const std::string& path) {
	for (auto& it : ignores) {
		std::regex e(it);
		std::smatch result;
		if (std::regex_search(path, result, e)) {
			return true;
		}
	}
	return false;
}

void PrintProgramInfo() {
	std::cerr << "FBDevColladaToFBMesh v1.0\nCollada mesh converter for Fastbird Engine.\n\n";
}

void PrintOptions(boost::program_options::options_description& desc) {
	std::cerr << desc << std::endl;
}

int main(int argc, char* argv[])
{
	boost::program_options::variables_map vm;
	StringVector args;
	try {
		boost::program_options::options_description options("Options");
		options.add_options()
			("exclude,e", boost::program_options::value<std::string>(), "Exclude file.")
			("date,d", "Check date.");

		if (argc == 1) {
			PrintProgramInfo();
			PrintOptions(options);
		}

		boost::program_options::command_line_parser parser(argc, argv);
		parser.options(options).allow_unregistered().style(boost::program_options::command_line_style::default_style |
			boost::program_options::command_line_style::allow_slash_for_short);
		auto parsed_options = parser.run();
		boost::program_options::store(parsed_options, vm);
		args = boost::program_options::collect_unrecognized(parsed_options.options, boost::program_options::include_positional);
		notify(vm);
	}
	catch (const boost::program_options::error& ex) {
		std::cerr << ex.what() << std::endl;		
		return 1;
	}

	std::string ignore_file;
	if (vm.count("exclude")) {
		ignore_file = vm["exclude"].as<std::string>();
		if (!FileSystem::Exists(ignore_file.c_str())) {
			std::cerr << "Exclude file not exists.\n";
			return 1;
		}
	}
	bool check_date = false;
	if (vm.count("date")) {
		check_date = true;
	}
	
	std::ifstream file(ignore_file);
	if (file) {
		while (file) {
			std::string item;
			file >> item;
			if (!item.empty())
				ignores.insert(item);
		}
	}

	std::string targetDir = "./";
	if (args.empty()) {
		args.push_back("./");
	}

	// test a file.
	/*save_meshes("Data_original/objects/modules/highlight.dae");
	MeshImportDesc desc;
	desc.keepMeshData = true;
	desc.mergeMaterialGroups = true;
	auto mesh = load_mesh("Data_original/objects/modules/highlight.fbmesh", desc);*/

	// do real job.
	for (auto& dir : args) {
		if (FileSystem::IsDirectory(dir.c_str())) {
			dir = FileSystem::AddEndingSlashIfNot(dir.c_str());
			auto it = FileSystem::GetDirectoryIterator(dir.c_str(), true);
			while (it->HasNext()) {
				bool is_dir;
				auto filepath = it->GetNextFilePath(&is_dir);
				if (is_dir)
					continue;
				if (!FileSystem::HasExtension(filepath, ".dae"))
					continue;

				if (ignore(filepath))
					continue;
				
				std::string fbmesh_path = FileSystem::ReplaceExtension(filepath, "fbmesh");
				std::string compared_mesh_path;
				int compare = FileSystem::FILE_NOT_EXISTS;
				if (check_date) {
					if (FileSystem::Exists(fbmesh_path.c_str())) {
						compare = FileSystem::CompareFileModifiedTime(filepath, fbmesh_path.c_str());
						compared_mesh_path = fbmesh_path;
					}
					if (compare != 1) {
						std::string fbmeshgroup_path = FileSystem::ReplaceExtension(filepath, "fbmesh_group");
						if (FileSystem::Exists(fbmeshgroup_path.c_str())) {
							compare = FileSystem::CompareFileModifiedTime(filepath, fbmeshgroup_path.c_str());
							compared_mesh_path = fbmeshgroup_path;
						}
					}
					if (compare != 1) {
						std::string fbmeshes_path = FileSystem::ReplaceExtension(filepath, "fbmeshes");
						if (FileSystem::Exists(fbmeshes_path.c_str())) {
							compare = FileSystem::CompareFileModifiedTime(filepath, fbmeshes_path.c_str());
							compared_mesh_path = fbmeshes_path;
						}
					}
					if (compare != 1) {
						std::string daedesc_path = FileSystem::ReplaceExtension(filepath, "dadesc");
						if (FileSystem::Exists(daedesc_path.c_str())) {
							compare = FileSystem::CompareFileModifiedTime(daedesc_path.c_str(), compared_mesh_path.c_str());

						}
					}
				}
				if (!check_date || compare > 0 ) {
					std::cout << "Converting : " << filepath << std::endl;
					if (!save_meshes(filepath)) {
						std::cerr << "Failed to save " << filepath << "\n";
					}
				}
			}
		}
	}
	return 0;
}

