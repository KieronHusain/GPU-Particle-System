cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

Texture2D	particleTexture		: register(t0);

SamplerState SampleType		: register(s0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

struct particles
{
	float3 position : POSITION;
	float3 velocity;
	float life;
	float age;
	bool alive;
	float startSize;
	float endSize;
	float4 startColour;
	float4 endColour;
	float4 currentColour;
	float currentSize;
};

struct VS_INPUT
{
	uint id : SV_VERTEXID;
};

struct GS_INPUT
{
    float3 Pos : POSITION;
	float4 Col : COLOUR;
	float Size : SIZE;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOUR;
	float2 TexCoord : TEXCOORD;
};

static float4 scalePos[4];

static const float2 texCoords[4] =
{
	float2(0, 1),
	float2(1, 1),
	float2(0, 0),
	float2(1, 0),
};

StructuredBuffer<particles>   particlesIn : register(t0);

GS_INPUT VS(VS_INPUT input)
{
	GS_INPUT output;

	uint index = input.id;
	particles p = particlesIn[index];

	output.Pos = p.position;

	output.Size = p.currentSize; 

	output.Col = p.currentColour;

	return output;
}

[maxvertexcount(4)]
void GS(point GS_INPUT input[1], inout TriangleStream<PS_INPUT> SpriteStream)
{
	PS_INPUT output;

	//Scales the size down so it isn't stretched by the aspect ratio 
	float scaleX = input[0].Size * 0.01f;
	float scaleY = input[0].Size * 0.01f * 1.77777f;

	//Points that make up a quad
	scalePos[0] = float4(-scaleX, scaleY, 0, 0);
	scalePos[1] = float4(scaleX, scaleY, 0, 0);
	scalePos[2] = float4(-scaleX, -scaleY, 0, 0);
	scalePos[3] = float4(scaleX, -scaleY, 0, 0);

	// Transform to view space
	float4 viewPosition = mul(float4(input[0].Pos, 1.0f), World);
	viewPosition = mul(viewPosition, View);
	viewPosition = mul(viewPosition, Projection);

	// Emit two new triangles
	for (int i = 0; i < 4; i++)
	{
		output.Pos = viewPosition + scalePos[i];

		output.TexCoord = texCoords[i];
		output.Col = input[0].Col;

		SpriteStream.Append(output);
	}

	SpriteStream.RestartStrip();
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 textureColor;
	
	textureColor = particleTexture.Sample(SampleType, input.TexCoord);
	
	//Clips the edges around texture to give the smoke particle shape
	clip(textureColor.xyz - 0.1f);
	
	textureColor.a = input.Col.a;

	return input.Col;
	//return textureColor;
}