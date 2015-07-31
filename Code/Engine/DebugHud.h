#pragma once

#include <Engine/Object.h>
#include <Engine/RendererStructs.h>
#include <../es/shaders/Constants.h>
#include <Engine/ISceneListener.h>

#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Color.h>
#undef DrawText

namespace fastbird
{

class IShader;
class DebugHud : public Object, public ISceneListener
{
public:
	DebugHud();
	virtual ~DebugHud();

	struct TextData
	{
		TextData(const Vec2I& pos, WCHAR* text, const Color& color, float size, float secs)
			: mPos(pos), mText(text), mColor(color), mSecs(secs), mSize(size), mDuration(secs)
		{
		}

		Vec2I mPos;
		std::wstring mText;
		Color mColor;
		float mSecs;
		float mDuration;
		float mSize;
	};

	struct Line
	{
		Vec3 mStart;
		unsigned mColor;
			
		Vec3 mEnd;
		unsigned mColore;
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

	/*struct HdrLine
	{
		Vec3 mStart;
		Color mColor;

		Vec3 mEnd;
		Color mColore;
	};*/

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
	void DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color, float size);
	void DrawText(const Vec2I& pos, WCHAR* text, const Color& color, float size);
	void Draw3DText(const Vec3& pos, WCHAR* text, const Color& color, float size);
	// if wolrdspace is false, it's in the screenspace 0~width, 0~height
	void DrawLine(const Vec3& start, const Vec3& end, const Color& color0,
		const Color& color1);
	void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0,
		const Color& color1);
	void DrawLine(const Vec2I& start, const Vec2I& end, const Color& color0, 
		const Color& color1);
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
	/*struct HDR_LINE_VERTEX
	{
		HDR_LINE_VERTEX(const Vec3& pos, Color c)
		: v(pos), color(c)
		{
		}
		Vec3 v;
		Color color;
	};*/
	static const unsigned LINE_STRIDE;
	/*static const unsigned HDR_LINE_STRIDE;*/
	static const unsigned MAX_LINE_VERTEX;
	typedef std::queue<TextData> MessageQueue;
	MessageQueue mTexts;
	typedef std::list<TextData> MessageBuffer;
	std::map<Vec2I, MessageBuffer> mTextsForDur;
	std::vector<Line> mScreenLines;
	std::vector<Line> mWorldLines;
	std::vector<Line> mWorldLinesBeforeAlphaPass;
	OBJECT_CONSTANTS mObjectConstants;
	OBJECT_CONSTANTS mObjectConstants_WorldLine;

	std::vector<Sphere> mSpheres;
	std::vector<Box> mBoxes;
	std::vector<Triangle> mTriangles;

	SmartPtr<IShader> mLineShader;
	SmartPtr<IInputLayout> mInputLayout;
	/*SmartPtr<IInputLayout> mHdrInputLayout;*/
	SmartPtr<IVertexBuffer> mVertexBuffer;
	/*SmartPtr<IVertexBuffer> mHdrVertexBuffer;*/
	SmartPtr<IMaterial> mTriMaterial;

	SmartPtr<RenderStates> mRenderStates;
	IMeshObject* mSphereMesh;
	IMeshObject* mBoxMesh;
};

}