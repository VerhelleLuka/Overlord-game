//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Mirror;
	AddressV = Mirror;
};

Texture2D gTexture;
float gBluriness;
/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState Depth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
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
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position, 1.f);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 color = {0.f,0.f,0.f,0.f};
// Step 1: find the dimensions of the texture (the texture has a method for that)	
float x, y;
gTexture.GetDimensions(x, y);
// Step 2: calculate dx and dy (UV space for 1 pixel)	
	float dx, dy;
	dx = 1.f / x;
	dy = 1.f / y;
	// Step 3: Create a double for loop (5 iterations each)
	for (int i = 0; i < (int)gBluriness; ++i)
	{
		for (int j = 0; j < (int)gBluriness; ++j)
		{
			//Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
			float xOffset;
			float yOffset;
			xOffset = i * dx * 2;
			yOffset = j * dy * 2;
			//Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
			color += gTexture.Sample(samPoint, float2(input.TexCoord.x + xOffset, input.TexCoord.y + yOffset));

		}
	}

	// Step 4: Divide the final color by the number of passes (in this case 5*5)	
	// Step 5: return the final color
	if (gBluriness != 0.f)
	color = color / (gBluriness * gBluriness);
	
	return color;
}


//TECHNIQUE
//---------
technique11 Blur
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