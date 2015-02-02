#pragma once

namespace fastbird
{
	class IDebugDrawer
	{
	public:
		virtual void DrawLine(const Vec3& from, const Vec3& to, const Color& color) = 0;
		virtual void DrawLine(const Vec3& from, const Vec3& to, const Color& fromColor, const Color& toColor) = 0;
		virtual void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness,
			const char* texture, bool textureFlow) = 0;

		virtual void	DrawSphere(const Vec3& p, float radius, const Color& color) = 0;
		virtual void	DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha) = 0;
		virtual void	DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha) = 0;
		virtual void  Draw3DText(const Vec3& location, const char* text) = 0;
	};
}