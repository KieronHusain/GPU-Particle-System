#ifndef __PARTICLE_EMITTER_SETTINGS_H__
#define __PARTICLE_EMITTER_SETTINGS_H__

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>

//Creates random number ranges for systems
struct Variances
{
	DirectX::XMFLOAT3 positionVariance;
	float pad;
	DirectX::XMFLOAT3 velocityVariance;
	float pad1;
	//float lifeVariance;
};

//Parameters for emitters, padded so they are aligned on the GPU
struct EmitterParameters
{
	int emitterType;
	int numToEmitPerFrame;
	float particleLife;
	float startSize;
	float endSize;
	float pad6;
	float pad4;
	float pad5;
	DirectX::XMFLOAT3 emitterPosition;
	float pad1;
	DirectX::XMFLOAT3 emitterVelocity;
	float pad;
	DirectX::XMFLOAT3 particlePosition;
	float pad2;
	DirectX::XMFLOAT3 particleVelocity;
	float pad3;
	DirectX::XMFLOAT4 startColour;
	DirectX::XMFLOAT4 endColour;
	DirectX::XMFLOAT4 currentColour;
	float currentSize;
	DirectX::XMFLOAT3 pad8;
};

//Individual particle properties
struct Particle
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	float life;
	float age;
	bool alive;
	float startSize;
	float endSize;
	DirectX::XMFLOAT4 startColour;
	DirectX::XMFLOAT4 endColour;
	DirectX::XMFLOAT4 currentColour;
	float currentSize;
};

#endif

