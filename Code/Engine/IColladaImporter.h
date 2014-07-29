#pragma once
namespace fastbird
{
	class IMeshObject;
	class MeshGroup;
	class CLASS_DECLSPEC_ENGINE IColladaImporter : public ReferenceCounter
	{
	public:
		static IColladaImporter* CreateColladaImporter();

		virtual ~IColladaImporter() {}
		// mergeMaterialGroups is used by voxelizer and modules.
		// if you use merging, you have to set material by yourself.
		virtual bool ImportCollada(const char* filepath, bool yzSwap_false, bool oppositeCull_true, 
			bool useIndexBuffer, bool mergeMaterialGroups, bool keepMeshData, bool generateTangent, bool meshGroup) = 0;
		virtual IMeshObject* GetMeshObject() const = 0;
		virtual IMeshObject* GetMeshObject(const char* id) const = 0;
		virtual IMeshGroup* GetMeshGroup() const = 0;

	};
}