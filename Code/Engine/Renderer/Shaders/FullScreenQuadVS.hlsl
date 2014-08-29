//---------------------------------------------------------------------------
struct QuadVS_Input
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
};

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// VS
//---------------------------------------------------------------------------
QuadVS_Output fullscreenquadvs_VertexShader(QuadVS_Input Input)
{
	QuadVS_Output Output;
	Output.Pos = float4(Input.Pos, 1.0);
	Output.Tex = Input.Tex;
	return Output;
}