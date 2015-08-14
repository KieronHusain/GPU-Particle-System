#include "resource.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h> 
#include <time.h>  
#include <string>
#include <d3d11.h>

#include "DirectXTK\Inc\SpriteBatch.h"
#include "DirectXTK\Inc\SpriteFont.h"

using namespace DirectX;

extern std::unique_ptr<SpriteBatch> g_Sprites;
extern std::unique_ptr<SpriteFont> g_Font;

Game::Game(ID3D11Device* GD, ID3D11DeviceContext* deviceContext, HINSTANCE hInstance)
{
	input = new Input(hInstance);

	//---------------------------------------------------GPU PARTICLE SYSTEM---------------------------------------------------------//
	gpuParticleSystem = new GPUParticleSystem(GD, deviceContext);

	//EMITTERS AND FORCEFIELDS GO HERE//
	//Emitter Type Key:
	//0 - Omni, 1 - VolumeBox, 2 - VolumeSphere, 3 - VolumeTorus, 4 - Curve

	//UNCOMMENT A BLOCK AND BUILD THE SYSTEM TO SEE THE EFFECT

	////////////////////////////////////////////

	//Fire and Smoke emitters effected by wind force
	/*gpuParticleSystem->addEmitter(1, DirectX::XMFLOAT3(0.0f, -3.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 1024, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f), 3.0f, 20.0f, 30.0f,
		DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.04f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.1f, 1.0f, 0.1f));
	gpuParticleSystem->addEmitter(1, DirectX::XMFLOAT3(0.0f, -3.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 512, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f), 7.0f, 20.0f, 40.0f,
		DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.03f), DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.2f, 1.0f, 0.2f));
	gpuParticleSystem->shaderFactory->AddGravityWindField(DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f));*/

	////////////////////////////////////////////

	//Smoke moving through a vortex
	//gpuParticleSystem->addEmitter(1, DirectX::XMFLOAT3(-10.0f, -3.0f, 0.0f), DirectX::XMFLOAT3(0.01f, 0.0f, 0.0f), 1024, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), 7.0f, 20.0f, 40.0f,
	//	DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.04f), DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.1f, 1.0f, 0.1f));
	//gpuParticleSystem->shaderFactory->AddVortexField(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 5.0f, 1000.0f, 5.0f);

	////////////////////////////////////////////

	//Torus Emitter with multiple radial fields
	gpuParticleSystem->addEmitter(3, DirectX::XMFLOAT3(0.0f, -5.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 1024, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 3.0f, 0.0f), 7.0f, 5.0f, 5.0f,
		DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.2f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f));
	gpuParticleSystem->shaderFactory->AddRadialField(DirectX::XMFLOAT3(5.0f, 3.0f, 0.0f), 2.0f, 0.5f);
	gpuParticleSystem->shaderFactory->AddRadialField(DirectX::XMFLOAT3(-5.0f, 3.0f, 0.0f), 2.0f, 0.5f);
	gpuParticleSystem->shaderFactory->AddRadialField(DirectX::XMFLOAT3(0.0f, 1.5f, -5.0f), 2.0f, 0.5f);
	gpuParticleSystem->shaderFactory->AddRadialField(DirectX::XMFLOAT3(0.0f, 1.5f, 5.0f), 2.0f, 0.5f);

	////////////////////////////////////////////

	//Curve emitter within a vortex
	//gpuParticleSystem->addEmitter(4, DirectX::XMFLOAT3(0.0f, -6.0f, -0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 1024, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f), 10.0f, 5.0f, 5.0f,
	//	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.2f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	//gpuParticleSystem->shaderFactory->AddVortexField(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 10.0f, 1000.0f, 1.0f);

	////////////////////////////////////////////

	//Sphere emitter with a gravitational field
	//gpuParticleSystem->addEmitter(2, DirectX::XMFLOAT3(0.0f, -2.0f, -0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 1024, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 6.0f, 0.0f), 10.0f, 5.0f, 5.0f,
	//	DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.2f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(2.0f, 1.0f, 2.0f));
	//gpuParticleSystem->shaderFactory->AddGravityWindField(DirectX::XMFLOAT3(0.0f, -3.0f, 0.0f));

	//-------------------------------------------------------------------------------------------------------------------------------//
	
	//Initialises the system
	gpuParticleSystem->InitialiseSystem(GD, deviceContext);

	FPS = new FpsClass();
	FPS->Initialize();

	testAdd = 0;

	//Describes blend state required
	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//Sets device up for alpha blending and fading out of particles over time
	GD->CreateBlendState(&transparentDesc, &transparentBlendState);
	deviceContext->OMSetBlendState(transparentBlendState, 0, 0xffffffff);
}

Game::~Game()
{
	//tidy myInput
	if (input)
	{
		delete input;
	}

	delete gpuParticleSystem;
	delete FPS;
}

bool Game::update(ID3D11Device* GD, ID3D11DeviceContext* deviceContext)
{
	//collect new input 
	input->Pole();

	//quit on pressing the Escape Key
	if (input->kState(DIK_ESCAPE) & 0x80)
	{
		delete gpuParticleSystem;
		delete FPS;
		return false;
	}

	using namespace std;

	//calculate frame time-step dt for passing down to game objects
	DWORD currentTime = GetTickCount();
	float  dt = min((float)(currentTime - playTime) / 1000.0f, 0.01f);
	playTime = currentTime;

	std::cout << dt << endl;

	//Gets the number of particles that are alive, need to change this for a release build
	testAdd = gpuParticleSystem->g_maxParticles - gpuParticleSystem->particleCounter;

	//Allow camera rotation around the particle system
	if (input->kState(DIK_LEFTARROW) & 0x80)
	{
		gpuParticleSystem->rot -= 1.0f * dt;
	}

	if (input->kState(DIK_RIGHTARROW) & 0x80)
	{
		gpuParticleSystem->rot += 1.0f * dt;
	}

	FPS->Frame();

	gpuParticleSystem->Update(GD, deviceContext, dt);


	return true;
}

//helper function to convert to odd char type used by Microsoft stuff
wchar_t* charToWChar(const char* text)
{
	size_t size = strlen(text) + 1;
	static wchar_t* wa = NULL;
	if (wa)
	{
		delete[] wa;
		wa = NULL;
	}
	wa = new wchar_t[size];
	mbstowcs(wa, text, size);
	return wa;
}

void Game::draw(ID3D11DeviceContext* GD)
{
	//All the rubbish to get the string into the right type
	std::string particlesAlive = std::to_string(testAdd);
	const char* convertCount = particlesAlive.c_str();

	std::string fpsString = std::to_string(FPS->GetFps() * 2);
	const char* convertFPS = fpsString.c_str();
	
	//Draw the particle system
	gpuParticleSystem->Draw(GD);

	g_Sprites->Begin(DirectX::SpriteSortMode_Deferred, transparentBlendState);
	
	g_Font->DrawString(g_Sprites.get(), L"FPS: ", XMFLOAT2(0, 0));
	g_Font->DrawString(g_Sprites.get(), L"Particles Alive: ", XMFLOAT2(0, 50));
	
	g_Font->DrawString(g_Sprites.get(), charToWChar(convertCount), XMFLOAT2(285, 50));
	g_Font->DrawString(g_Sprites.get(), charToWChar(convertFPS), XMFLOAT2(95, 0));

	g_Sprites->End();
}