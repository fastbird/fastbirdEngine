#pragma once
// implementation of double connected edge list

#include <CommonLib/Math/Vec2.h>
#include <vector>
#include <assert.h>
namespace fastbird{

	struct dcelVertex2{
		Vec2 mCoord;
		unsigned mIncidentEdge;
	};

	struct dcelFace2 {
		// edge
		unsigned mOuterComponent;
		unsigned mInnerComponent;
	};

	struct dcelHalfEdge2 {
		// vertex
		unsigned mOrigin;

		// edge
		unsigned mTwin;		
		unsigned mNext;
		unsigned mPrev;

		// face
		unsigned mIncidentFace;

		bool mOpposite;
	};

	struct DCEL2 {
		std::vector<dcelVertex2> mVertices;
		std::vector<dcelFace2> mFaces;
		std::vector<dcelHalfEdge2> mHalfEdges;

		void AddVertex(const Vec2& v, bool closing = false){
			dcelVertex2* newVertex = 0;
			unsigned prevVertexIdx = -1;			
			unsigned newVertexIdx = -1;
			if (!closing){
				mVertices.push_back(dcelVertex2());
				newVertex = &mVertices.back();
				newVertex->mCoord = v;
				newVertex->mIncidentEdge = -1;
				newVertexIdx = mVertices.size() - 1;
				prevVertexIdx = mVertices.size() - 2;
			}
			else{
				if (mVertices.empty()){
					assert(0);
					return;
				}
				newVertex = &mVertices[0];
				newVertexIdx = 0;
				prevVertexIdx = mVertices.size() - 1;
			}		
			
			auto numVertices = mVertices.size();
			if (numVertices > 1){
				auto& prevV = mVertices[prevVertexIdx];
				mHalfEdges.push_back(dcelHalfEdge2());				
				auto& halfEdge = mHalfEdges.back();
				unsigned halfEdgeIndex = mHalfEdges.size() - 1;
				halfEdge.mOpposite = false;
				halfEdge.mOrigin = prevVertexIdx;
				if (prevV.mIncidentEdge == -1){
					prevV.mIncidentEdge = halfEdgeIndex;
				}
				halfEdge.mNext = -1;				
				halfEdge.mPrev = -1;
				// check prev
				if (numVertices > 2){
					unsigned prevEdgeOriginIdx = prevVertexIdx - 1;
					for (unsigned i = halfEdgeIndex - 1; i != -1; --i){
						if (mHalfEdges[i].mOrigin == prevEdgeOriginIdx && !mHalfEdges[i].mOpposite){
							halfEdge.mPrev = i;
							mHalfEdges[i].mNext = halfEdgeIndex;
							break;
						}
					}
				}
				if (halfEdge.mPrev == -1){
					// create face
					mFaces.push_back(dcelFace2());
					unsigned faceIdx = mFaces.size() - 1;
					halfEdge.mIncidentFace = faceIdx;
					mFaces[faceIdx].mInnerComponent = halfEdgeIndex;
					mFaces[faceIdx].mOuterComponent = -1;
				}
				else{
					halfEdge.mIncidentFace = mHalfEdges[halfEdge.mPrev].mIncidentFace;
				}
				halfEdge.mIncidentFace = -1;			

				mHalfEdges.push_back(dcelHalfEdge2());
				auto& twin = mHalfEdges.back();
				unsigned twinIndex = mHalfEdges.size() - 1;
				halfEdge.mTwin = twinIndex;
				twin.mTwin = halfEdgeIndex;
				twin.mOpposite = true;
				twin.mOrigin = newVertexIdx;
				twin.mNext = -1;				
				// check next
				unsigned prevEdgeOriginIdx = prevVertexIdx;
				for (unsigned i = twinIndex - 1; i != -1; --i){
					if (mHalfEdges[i].mOrigin == prevEdgeOriginIdx && mHalfEdges[i].mOpposite){
						twin.mNext = i;
						mHalfEdges[i].mPrev = twinIndex;
						break;
					}
				}
				if (twin.mNext == -1){
					//create face
					mFaces.push_back(dcelFace2());
					unsigned faceIdx = mFaces.size() - 1;
					twin.mIncidentFace = faceIdx;
					mFaces[faceIdx].mInnerComponent = -1;
					mFaces[faceIdx].mOuterComponent = twinIndex;
				}
				else{
					twin.mIncidentFace = mHalfEdges[twin.mNext].mIncidentFace;
				}

				if (closing){
					unsigned prevOriginIdx = 1;
					for (unsigned i = 0; i < 2; ++i){
						if (mHalfEdges[i].mOrigin == prevOriginIdx && mHalfEdges[i].mOpposite){
							twin.mPrev = i;
							mHalfEdges[i].mNext = twinIndex;
							break;
						}
					}
				}
				else{
					twin.mPrev = -1;
				}
				twin.mIncidentFace = -1;			

			}
		}

		// link the last vertex to the first one
		void CloseList(){
			if (mVertices.size() < 3)
				return;
			AddVertex(Vec2::ZERO, true);
		}
	};
}