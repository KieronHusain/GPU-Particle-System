#include "ShaderFactory.h"

ShaderFactory::ShaderFactory()
{

}

ShaderFactory::~ShaderFactory()
{

}

//Creates the final shader code for the update shader
void ShaderFactory::ShaderAssembly()
{
	std::string baseStart =
		"cbuffer PerFrameVariables : register(b0)\n"
		"{\n"
		"	float deltaTime;\n"
		"}\n"
		"cbuffer EmitterParameters : register(b1)"
		"{"
		"	int emitterType;"
		"	int numToEmitPerFrame;"
		"	float particleLife;"
		"	float startSize;"
		"	float endSize;"
		"	float pad6;"
		"	float pad4;"
		"	float pad5;"
		"	float3 emitterPosition;"
		"	float pad1;"
		"	float3 emitterVelocity;"
		"	float pad;"
		"	float3 particlePosition;"
		"	float pad2;"
		"	float3 particleVelocity;"
		"	float pad3;"
		"	float4 startColour;"
		"	float4 endColour;"
		"}"
		"struct particles\n"
		"{\n"
		"	float3 position;\n"
		"	float3 velocity;\n"
		"	float life;\n"
		"	float age;\n"
		"	bool alive;\n"
		"	float startSize;\n"
		"	float endSize;\n"
		"	float4 startColour;\n"
		"	float4 endColour;\n"
		"	float4 currentColour;\n"
		"	float currentSize;\n"
		"};"
		"RWStructuredBuffer<particles>	particleBuffer		: register(u0);\n"
		"AppendStructuredBuffer<uint>			g_DeadListToAddTo		: register(u1);\n"
		"[numthreads(256, 1, 1)]\n"
		"void main(uint3 DTid : SV_DispatchThreadID)\n"
		"{\n"
		"	particles p = (particles)0;\n"
		"	p = particleBuffer[DTid.x];\n"
		"	if (p.age > 0.0f)\n"
		"	{\n"
		"		float3 newPos = 0;\n"
		"		float3 newVel = 0;\n"
		"		float forceX = 0;\n"
		"		float forceY = 0;\n"
		"		float forceZ = 0;\n"
		"		float tempRadius = 0;\n"
		"		newPos.xyz = p.position.xyz;\n"
		"		newVel.xyz = p.velocity.xyz;\n";
	std::string baseEnd =
		"		float scaledLife = 1.0 - saturate(p.age / p.life);"
		"		float4 color0 = p.startColour;"
		"		float4 color1 = p.endColour;"
		"		p.currentColour = lerp(color0, color1, saturate(scaledLife)).xyzw;"
		"		float size0 = p.startSize;"
		"		float size1 = p.endSize;"
		"		p.currentSize = lerp(size0, size1, saturate(scaledLife));"
		"       p.age -= deltaTime;\n"
		"		float3 newEmitterPos = emitterVelocity * deltaTime;"
		"		newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;\n"

		"		p.position.xyz = newPos.xyz;\n"
		"		p.velocity.xyz = newVel.xyz;\n"
		"	}\n"
		"	if (p.age <= 0.0f && p.alive == true)\n"
		"	{\n"
		"		g_DeadListToAddTo.Append(DTid.x);\n"
		"		p.life = -1.0f;\n"
		"		p.alive = false;\n"
		"		p.position.xyz = 9999.0f;\n"
		"		p.velocity.xyz = 0.0f;\n"
		"		p.startSize = 1.0f;\n"
		"		p.startColour = float4(1.0f, 0.0f, 1.0f, 1.0f);\n"
		"		p.endColour = float4(0.0f, 0.0f, 0.0f, 1.0f);\n"
		"	}\n"
		"	particleBuffer[DTid.x] = p;\n"
		"}\n";
	
	std::string forceFields;

	//Collects all of the desired gravity force fields
	for (int i = 0; i < gravityFields.size(); ++i)
	{
		std::string temp = gravityFields[i];
		forceFields = forceFields + temp;
	}
	
	//Collects all of the desired forcefields into a single string
	for (int i = 0; i < listOfStrings.size(); ++i)
	{
		std::string temp = listOfStrings[i];
		forceFields = forceFields + temp;
	}

	updateShaderCode = baseStart + forceFields + baseEnd;
}

void ShaderFactory::AddVortexField(DirectX::XMFLOAT3 position, float radius, float height, float speed)
{
	std::string vortexField =
		"if ((p.position.x-(" + Convert(position.x) + "))*(p.position.x - (" + Convert(position.x) + ")) +"
		"(p.position.z - (" + Convert(position.z) + "))*(p.position.z - (" + Convert(position.z) + ")) <("
		+ Convert(radius) + ")* (" + Convert(radius) + ") && p.position.y < (" + Convert(position.y) +
		")+((" + Convert(height) + ")/2) && p.position.y > (" + Convert(position.y) + ")-((" + Convert(height) + ")/2))\n"
		"{"
		"float dx = p.position.x -(" + Convert(position.x) + ");"
		"float dz = p.position.z -(" + Convert(position.z) + ");"
		"float vx = -dz*" + Convert(speed) + ";"
		"float vz = dx*" + Convert(speed) + ";"
		"float factor = 1.0f / (1.0f + (dx*dx + dz*dz) / 1.0f);"
		"float xVortex = (vx - newVel.x)*factor;"
		"float zVortex = (vz - newVel.z)*factor;"
		"newVel.xyz = float3(p.velocity.x + xVortex, p.velocity.y, p.velocity.z + zVortex);"
		"}";

	listOfStrings.push_back(vortexField);
}

void ShaderFactory::AddGravityWindField(DirectX::XMFLOAT3 force)
{
	std::string windGravityField =
		"newVel.xyz = float3(p.velocity.x + ((" + Convert(force.x) + 
		") * deltaTime), p.velocity.y + ((" + Convert(force.y) + 
		") * deltaTime), p.velocity.z + ((" + Convert(force.z) + ") * deltaTime));";

	gravityFields.push_back(windGravityField);
}

void ShaderFactory::AddRadialField(DirectX::XMFLOAT3 position, float radius, float forceOutwards)
{
	std::string radialField =
		"tempRadius =(" + Convert(radius) + ");"
		"if ((p.position.x -(" + Convert(position.x) + "))*(p.position.x - (" + Convert(position.x) + 
		")) + (p.position.y - (" + Convert(position.y) + "))" 
		"*(p.position.y - (" + Convert(position.y) + ")) + (p.position.z - (" + Convert(position.z) + 
		"))*(p.position.z - (" + Convert(position.z) + ")) < tempRadius * (" + Convert(radius) + "))\n"
		"{\n"
		"	forceX = ((" + Convert(position.x) +  ")- p.position.x)*(" + Convert(forceOutwards) + ");\n"
		"	forceY = ((" + Convert(position.y) + ") - p.position.y)*(" + Convert(forceOutwards) + ");\n"
		"	forceZ = ((" + Convert(position.z) + ") - p.position.z)*(" + Convert(forceOutwards) + ");\n"
		"	newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;\n"
		"	newVel.xyz = float3(p.velocity.x - forceX * 0.1f, p.velocity.y - forceY * 0.1f, p.velocity.z - forceZ * 0.1f);\n"
		"}\n";

	listOfStrings.push_back(radialField);
}

//Helper function for converting floats to strings
std::string ShaderFactory::Convert(float number)
{
	std::ostringstream buff;
	buff << number;
	std::string s(buff.str());
	return s;
}