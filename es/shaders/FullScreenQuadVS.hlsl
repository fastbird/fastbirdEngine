//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// VS
//---------------------------------------------------------------------------
QuadVS_Output fullscreenquadvs_VertexShader(uint id : SV_VertexID)
{
	QuadVS_Output Output;
	Output.Pos.x = (float)(id/2) * 4.0 - 1.0;
	Output.Pos.y = (float)(id%2) * 4.0 - 1.0;
#ifdef _FAR_SIDE_QUAD
	Output.Pos.z = 1.0;
#else
	Output.Pos.z = 0.0;
#endif
	Output.Pos.w = 1.0;
	
	Output.Tex.x = (float)(id/2) * 2.0;
	Output.Tex.y = 1.0 - (float)(id%2) * 2.0;
	
	return Output;
}