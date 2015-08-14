//GAME CLASS

#ifndef _GAME_H_
#define _GAME_H_

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>
#include <iostream>
#include "input.h"
#include "GPUParticleSystem.h"

#include "FPS.h"

using namespace std;

class Game
{
public:
	Game(ID3D11Device* GD, ID3D11DeviceContext* deviceContext, HINSTANCE hInstance);
	~Game();

	bool update(ID3D11Device* GD, ID3D11DeviceContext* deviceContext);
	virtual void draw(ID3D11DeviceContext* GD);
	
	DWORD playTime;

	Input* input;

	GPUParticleSystem* gpuParticleSystem;

	int testAdd;

	FpsClass* FPS;

	ID3D11BlendState* transparentBlendState;

private:
};

#endif