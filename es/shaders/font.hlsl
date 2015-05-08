//----------------------------------------------------------------------------
// File: font.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
Texture2D  gFontTexture : register(t0);
//{
	//Filter = MIN_MAG_MIP_POINT;
//};

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
	float2  uv		: TEXCOORD;       //  8
	int4  channel   : BLENDINDICES;    //  4
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD0;
    int4   channel  : TEXCOORD1;  
};

//----------------------------------------------------------------------------
v2p font_VertexShader( in a2v IN )
{
    v2p OUT;

	OUT.pos = mul(gWorldViewProj, IN.pos);
	OUT.color = IN.color;
	OUT.uv = IN.uv;
	OUT.channel = IN.channel;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 font_PixelShader( in v2p IN ) : SV_Target
{
	float4 color = gFontTexture.Sample(gLinearSampler, IN.uv);
	if (color.a<0.1f)
		discard;
	if( dot(vector<int, 4>(1,1,1,1), IN.channel) )
    {
        // Get the pixel value
		float val = dot(color, IN.channel);
		
        // A value above .5 is part of the actual character glyph
        // A value below .5 is part of the glyph outline
		color.rgb = val > 0.5 ? 2*val-1 : 0;
		color.a   = val > 0.5 ? 1 : 2*val;
    }
	color *= IN.color;
	return float4(IN.color.rgb, color.a);
}