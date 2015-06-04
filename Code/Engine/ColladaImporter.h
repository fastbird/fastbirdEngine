#pragma once
#include <Engine/IColladaImporter.h>
#include <Engine/AnimationData.h>
#include <Engine/CollisionInfo.h>
#include <CommonLib/FBColShape.h>
#include <COLLADAFWIWriter.h>
namespace COLLADAFW
{
	class Mesh;
	class MeshVertexData;
	class Node;
	class FloatOrDoubleArray;
}
namespace fastbird
{
	class IMeshObject;
	class IMeshGroup;
	typedef std::vector< std::pair<std::string, Transformation> > AUXILIARIES;

	// Write collada data to our format.
	class ColladaImporter : public IColladaImporter, public COLLADAFW::IWriter
	{
		typedef std::vector<float> FLOAT_DATA;
		MeshObjects mMeshObjects;
		MeshObjects mCollisionMeshes;
		SmartPtr<IMeshGroup> mMeshGroup;
		std::string mFilepath;
		bool mSwapYZ;
		bool mOppositeCull;
		bool mUseIndexBuffer;
		bool mMergeMaterialGroups;
		bool mGenerateTangent;
		bool mUseMeshGroup;
		bool mKeepMeshdata;

		/*size_t mNumMeshes;
		std::vector<std::string> mNames;
		std::vector<std::string> mIDs;
		std::vector<FLOAT_DATA> mPos;*/
		typedef std::vector<UINT> INDICES;
		typedef std::vector< INDICES > INDICES_PRIMITIVES;// indices per primitives
		//std::vector< INDICES_PRIMITIVES > mPosIndices; // indices per mesh
		//std::vector<FLOAT_DATA> mNormals;
		//std::vector< INDICES_PRIMITIVES > mNormalIndices; // indices per mesh
		//std::vector< FLOAT_DATA > mUVs;
		//std::vector< INDICES_PRIMITIVES > mUVIndices; // indices per mesh
		typedef std::vector< SmartPtr<IMaterial> > MATERIALS_PRIMITIVES; // materials per primitives
		//std::vector< MATERIALS_PRIMITIVES > mMaterials; // materials per mesh

		AUXILIARIES mAuxil;

		struct MeshInfo
		{
			std::string mName;
			std::string mUniqueId;
			FLOAT_DATA mPos;
			FLOAT_DATA mNormals;
			FLOAT_DATA mUVs;
			std::vector<bool> mHasUVs;
			int mNumPrimitives;
			// per primitives
			INDICES_PRIMITIVES mPosIndices;
			INDICES_PRIMITIVES mNormalIndices;
			INDICES_PRIMITIVES mUVIndices;
			MATERIALS_PRIMITIVES mMaterials;

		};
		std::vector<MeshInfo> mMeshInfos;

		COLLISION_INFOS mCollisions;
		std::map<std::string, AnimationData> mAnimData;

		//------------------------------------------------------------------
		// Private functions
		//------------------------------------------------------------------
		ColladaImporter(const ColladaImporter& other);
		const ColladaImporter& operator= (const ColladaImporter& other);
		MeshInfo* CopyData(COLLADAFW::Mesh* pColladaMesh);
		void GetFloatOrDouble(FLOAT_DATA& dest, COLLADAFW::FloatOrDoubleArray& src);
		IMeshObject* FeedGeometry(MeshInfo* meshInfo);
		IMeshObject* FeedGeometry_Collision(MeshInfo* meshInfo);
		IIndexBuffer* CreateIndexBuffer(UINT* indices, size_t num);
		void WriteChildNode(const COLLADAFW::Node* node, size_t parent);

	public:
		ColladaImporter();
		virtual ~ColladaImporter();

	protected:
		virtual void FinishSmartPtr();

	public:

		bool ImportCollada(const char* filepath, bool yzSwap, bool oppositeCull, 
			bool useIndexBuffer, bool mergeMaterialGroups, bool keepMeshData, bool generateTangent, bool meshGroup);
		virtual IteratorWrapper<MeshObjects> GetMeshIterator();
		virtual IMeshObject* GetMeshObject() const;
		virtual IMeshObject* GetMeshObject(const char* id) const;
		virtual IMeshGroup* GetMeshGroup() const { return mMeshGroup; }

		/** This method will be called if an error in the loading process occurred and the loader cannot
		continue to to load. The writer should undo all operations that have been performed.
		@param errorMessage A message containing informations about the error that occurred.
		*/
		virtual void cancel(const COLLADAFW::String& errorMessage);

		/** This is the method called. The writer hast to prepare to receive data.*/
		virtual void start();

		/** This method is called after the last write* method. No other methods will be called after this.*/
		virtual void finish();

        /** When this method is called, the writer must write the global document asset.
        @return The writer should return true, if writing succeeded, false otherwise.*/
        virtual bool writeGlobalAsset ( const COLLADAFW::FileInfo* asset );

        /** When this method is called, the writer must write the scene.
        @return The writer should return true, if writing succeeded, false otherwise.*/
        virtual bool writeScene ( const COLLADAFW::Scene* scene );

		/** When this method is called, the writer must write the entire visual scene.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeVisualScene ( const COLLADAFW::VisualScene* visualScene );

		/** When this method is called, the writer must handle all nodes contained in the 
		library nodes.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeLibraryNodes ( const COLLADAFW::LibraryNodes* libraryNodes );

		/** When this method is called, the writer must write the geometry.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeGeometry ( const COLLADAFW::Geometry* geometry );

		/** When this method is called, the writer must write the material.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeMaterial( const COLLADAFW::Material* material );

		/** When this method is called, the writer must write the effect.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeEffect( const COLLADAFW::Effect* effect );

		/** When this method is called, the writer must write the camera.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeCamera( const COLLADAFW::Camera* camera );

		/** When this method is called, the writer must write the image.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeImage( const COLLADAFW::Image* image );

		/** When this method is called, the writer must write the light.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeLight( const COLLADAFW::Light* light );

		/** When this method is called, the writer must write the Animation.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeAnimation( const COLLADAFW::Animation* animation );

		/** When this method is called, the writer must write the AnimationList.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeAnimationList( const COLLADAFW::AnimationList* animationList );

		/** When this method is called, the writer must write the skin controller data.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeSkinControllerData( const COLLADAFW::SkinControllerData* skinControllerData );

		/** When this method is called, the writer must write the controller.
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeController( const COLLADAFW::Controller* controller );

        /** When this method is called, the writer must write the formulas. All the formulas of the entire
		COLLADA file are contained in @a formulas.
        @return The writer should return true, if writing succeeded, false otherwise.*/
        virtual bool writeFormulas( const COLLADAFW::Formulas* formulas );

		/** When this method is called, the writer must write the kinematics scene. 
		@return The writer should return true, if writing succeeded, false otherwise.*/
		virtual bool writeKinematicsScene( const COLLADAFW::KinematicsScene* kinematicsScene );		
	};
}