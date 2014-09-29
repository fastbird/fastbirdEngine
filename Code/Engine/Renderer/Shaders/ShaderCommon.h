#ifndef _ShaderCommon_h_
#define _ShaderCommon_h_

#ifdef CPP

#include <CommonLib/Math/Mat44.h>
#include <CommonLib/Math/Vec4.h>

#define cbuffer struct
#define float4x4 Mat44
#define float4 Vec4
#define float3 Vec3
#define float2 Vec2

#else // !CPP

#pragma pack_matrix( row_major )
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define fract frac
#define mix lerp

//----------------------------------------------------------------------------
// SamplerState
// defined RendererEnum.h - SAMPLERS
//----------------------------------------------------------------------------
SamplerState gPointSampler : register(s0);
SamplerState gLinearSampler : register(s1);
SamplerState gAnisotropicSampler : register(s2);
SamplerComparisonState gShadowSampler : register(s3);
SamplerState gPointWrapSampler : register(s4);
SamplerState gLinearWrapSampler : register(s5);

#endif // !CPP


#endif //_ShaderCommon_h_