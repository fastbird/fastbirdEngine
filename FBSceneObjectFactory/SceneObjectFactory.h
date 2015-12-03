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
#include "FBCommonHeaders/Types.h"
#include "MeshImportDesc.h"
namespace fb{
	FB_DECLARE_SMART_PTR(TrailObject);
	FB_DECLARE_SMART_PTR(DustRenderer);
	FB_DECLARE_SMART_PTR(BillboardQuad);
	FB_DECLARE_SMART_PTR(SkySphere);
	FB_DECLARE_SMART_PTR(SkyBox);
	FB_DECLARE_SMART_PTR(MeshGroup);
	FB_DECLARE_SMART_PTR(MeshObject);
	FB_DECLARE_SMART_PTR(SceneObjectFactory);
	class FB_DLL_SCENEOBJECTFACTORY SceneObjectFactory{
		FB_DECLARE_PIMPL_NON_COPYABLE(SceneObjectFactory);	
		SceneObjectFactory();
		~SceneObjectFactory();

	public:
		/// You have the ownership of the returned pointer.
		static SceneObjectFactoryPtr Create();		
		static SceneObjectFactory& GetInstance();

		/// Set false to skip loading meshes.
		void SetEnableMeshLoad(bool enable);
		/** Creates a MeshObject with .dae file.
		This function creates a MeshObject from the .dae file and returned the cloned version of it.
		The original create version(archetype) will be preserved. Whenever you requested the same
		.dae file for another mesh object, the new mesh will be cloned from the archetype. */
		MeshObjectPtr CreateMeshObject(const char* daeFilePath);
		MeshObjectPtr CreateMeshObject(const char* daeFilePath, const MeshImportDesc& desc);
		/// Create mesh objects.
		/// Use this function for loading seperated meshes in a .dae file.
		/// Currently using for fracture meshes.
		std::vector<MeshObjectPtr> CreateMeshObjects(const char* daeFilePath, const MeshImportDesc& desc);
		/** Get an archetype mesh already loaded. */
		MeshObjectConstPtr GetMeshArcheType(const char* name);

		MeshGroupPtr CreateMeshGroup(const char* file);
		MeshGroupPtr CreateMeshGroup(const char* file, const MeshImportDesc& desc);

		SkySpherePtr CreateSkySphere();
		SkyBoxPtr CreateSkyBox(const char* materialPath);
		BillboardQuadPtr CreateBillboardQuad();
		DustRendererPtr CreateDustRenderer();
		TrailObjectPtr CreateTrailObject();

		void UpdateEnvMapInNextFrame(SkySpherePtr sky);

		void Update(TIME_PRECISION dt);
	};
}