#pragma once

#include <Engine/Foundation/Object.h>

namespace fastbird
{
	class ISkyBox : public Object
	{
	public:
		static ISkyBox* CreateSkyBoxInstance();

		enum SKYBOX_PLANE
		{
			SKYBOX_PLANE_FRONT,
			SKYBOX_PLANE_BACK,
			SKYBOX_PLANE_LEFT,
			SKYBOX_PLANE_RIGHT,
			SKYBOX_PLANE_UP,
			SKYBOX_PLANE_DOWN
		};
		virtual ~ISkyBox(){}

		virtual void Init() = 0;
	};
}