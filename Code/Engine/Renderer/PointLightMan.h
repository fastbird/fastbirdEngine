#pragma once


namespace fastbird
{
	struct POINT_LIGHT_CONSTANTS;
	class IPointLight;
	class PointLightMan
	{
		typedef std::vector< IPointLight* > PointLights;
		PointLights mPointLights;

	public:
		PointLightMan();
		~PointLightMan();

		IPointLight* CreatePointLight(const Vec3& pos, float range, const Vec3& color, float intensity, float lifeTime, bool manualDeletion);
		void DeletePointLight(IPointLight* pointLight);
		void Update(float dt);

		void GatherPointLightData(BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst);

	};
}