//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Mirror;
	AddressV = Mirror;
};

TextureCube gTexture : CubeMap;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState Depth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};
/// Create Rasterizer State (Backface culling) 

RasterizerState BackCulling
{
	CullMode = NONE;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
	float3 Position : POSITION;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.TexCoord = input.Position;
	output.Position = mul(float4(input.Position, 0.0f), gWorldViewProj).xyww;

	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	
	return gTexture.Sample(samPoint, input.TexCoord);
}


//TECHNIQUE
//---------
technique11 Default
{
	pass P0
	{
		SetRasterizerState(BackCulling);
		SetDepthStencilState(Depth, 0);
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}

