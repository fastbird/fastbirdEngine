#pragma once

#include <Engine/Object.h>
#include <Engine/RendererStructs.h>
#include <../es/shaders/Constants.h>
#include <Engine/ISceneListener.h>

#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Color.h>

namespace fastbird
{

	class IShader;
	class GeometryRenderer : public Object, public ISceneListener
	{
	public:
		GeometryRenderer();
		virtual ~GeometryRenderer();

		struct Line
		{
			Vec3 mStart;
			unsigned mColor;

			Vec3 mEnd;
			unsigned mColore;
		};

		struct ThickLine
		{
			Vec3 mStart;
			unsigned mColor;
			Vec3 mEnd;
			unsigned mColore;

			float mThickness;
			std::string mStrTexture;
			SmartPtr<ITexture> mTexture;
			bool mTextureFlow;
		};

		struct Sphere
		{
			Vec3 mPos;
			float mRadius;
			Color mColor;
		};

		struct Box
		{
			Vec3 mPos;
			Vec3 mExtent;
			Color mColor;
			float mAlpha;
		};

		struct Triangle
		{
			Vec3 a;
			Vec3 b;
			Vec3 c;
			Color mColor;
			float mAlpha;
		};

		//--------------------------------------------------------------------
		// IObject
		//--------------------------------------------------------------------
		virtual void Render();
		virtual void PreRender();
		virtual void PostRender();

		//--------------------------------------------------------------------
		// ISceneListener
		//--------------------------------------------------------------------
		virtual void OnBeforeRenderingTransparents(IScene* scene);

		//--------------------------------------------------------------------
		// Own
		//--------------------------------------------------------------------
		// if wolrdspace is false, it's in the screenspace 0~width, 0~height
		void DrawLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1);
		void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1);
		void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness, 
			const char* texture, bool textureFlow);
		void DrawSphere(const Vec3& pos, float radius, const Color& color);
		void DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha);
		void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha);

	private:

		struct LINE_VERTEX
		{
			LINE_VERTEX(const Vec3& pos, unsigned c)
			: v(pos), color(c)
			{
			}
			Vec3 v;
			unsigned color;
		};		
		static const unsigned LINE_STRIDE;

		struct THICK_LINE_VERTEX
		{
			THICK_LINE_VERTEX(const Vec3& pos, unsigned c, const Vec2& texcoord)
			: v(pos), color(c), uv(texcoord)
			{
			}
			Vec3 v;
			unsigned color;
			Vec2 uv;
		};
		static const unsigned THICK_LINE_STRIDE;

		static const unsigned MAX_LINE_VERTEX;
		std::vector<Line> mWorldLines;
		std::vector<Line> mWorldLinesBeforeAlphaPass;
		std::vector<ThickLine> mThickLines;
		OBJECT_CONSTANTS mObjectConstants;
		OBJECT_CONSTANTS mObjectConstants_WorldLine;

		std::vector<Sphere> mSpheres;
		std::vector<Box> mBoxes;
		std::vector<Triangle> mTriangles;

		SmartPtr<IShader> mLineShader;		
		SmartPtr<IInputLayout> mInputLayout;

		SmartPtr<IMaterial> mThickLineMaterial;

		SmartPtr<IVertexBuffer> mVertexBuffer;
		SmartPtr<IVertexBuffer> mVertexBufferThickLine;
		SmartPtr<IMaterial> mTriMaterial;

		SmartPtr<IDepthStencilState> mDepthStencilState;
		SmartPtr<IRasterizerState> mRasterizerState;

		IMeshObject* mSphereMesh;
		IMeshObject* mBoxMesh;
	};

}