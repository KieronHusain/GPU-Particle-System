//constant buffer for the shader
#ifndef _CONSTBUFFER_H_
#define _CONSTBUFFER_H_

#include <windows.h>
#include <directxmath.h>

using namespace DirectX;

//as passing to GPU needs to be correctly memory aligned
__declspec(align(16))
struct ConstantBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
};

#endif