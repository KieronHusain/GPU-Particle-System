//constant buffer for the shader
#ifndef _CONSTBUFFER_H_
#define _CONSTBUFFER_H_

#include <DirectXMath.h>

//as passing to GPU needs to be correctly memory aligned
__declspec(align(16))
struct MatricesConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
};

__declspec(align(16))
struct PerFrameVariables
{
	float deltaTime;
	float elapsedTime;
};

__declspec(align(16))
struct ParticleCount
{
	UINT particleCount;
};

__declspec(align(16))
struct DeadListCount
{
	UINT deadCount;
};

#endif