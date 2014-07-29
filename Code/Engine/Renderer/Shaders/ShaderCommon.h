#ifdef CPP

#include <CommonLib/Math/Mat44.h>
#include <CommonLib/Math/Vec4.h>

#define cbuffer struct
#define float4x4 Mat44
#define float4 Vec4
#define float3 Vec3
#define float2 Vec2

#else

#pragma pack_matrix( row_major )


#endif