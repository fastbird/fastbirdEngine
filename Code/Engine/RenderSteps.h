#pragma once

namespace fastbird
{
	namespace RenderSteps
	{
		enum Enum {
				Glow,
				ShadowMap,
				Depth,
				CloudVolume,
				GodRay,

				HDR, //including brightpass, lens flare, bloom

				Silouette,
				Num,
		};
	}
}