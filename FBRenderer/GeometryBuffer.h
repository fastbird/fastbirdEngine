#pragma once
#include "FBCommonHeaders/Types.h"
#include "PrimitiveTopology.h"
#include "FBRenderer/IndexBuffer.h" // Intentional
namespace fb {
	FB_DECLARE_SMART_PTR(VertexBuffer);	
	class Renderer;
	class FB_DLL_RENDERER GeometryBuffer {
	public:
		/// Create sphere vertex buffer.
		/// \param numRings Vertical(PI) split. 
		/// \param numSegments Horizontal(TWO_PI) split.
		/// will be PI / \a numLat_pi
		static GeometryBuffer CreateSphereMesh(Real radius, int numRings, int numSegments);

		VertexBufferPtr mVertexBuffer;
		IndexBufferPtr mIndexBuffer;		
		PRIMITIVE_TOPOLOGY mTopology;

		bool Bind();
	};
}
