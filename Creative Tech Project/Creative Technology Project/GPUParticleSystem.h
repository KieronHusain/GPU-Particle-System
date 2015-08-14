#ifndef __GPU_PARTICLE_SYSTEM_H__
#define __GPU_PARTICLE_SYSTEM_H__

#include <windows.h>
#include <d3d11_1.h>
#include <string>
#include <DirectXMath.h>
#include <vector>
#include "DirectXTK\Inc\WICTextureLoader.h"

#include "MatricesConstBuffer.h"
#include "ParticleEmitterSettings.h"
#include "ShaderFactory.h"

class GPUParticleSystem
{
public:

	GPUParticleSystem(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~GPUParticleSystem();

	//ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	//Constant Buffers
	int g_maxParticles;
	ID3D11Buffer* mConstBuffer;
	ID3D11Buffer* perFrameVariablesBuffer;
	ID3D11Buffer* emitterVariablesBuffer;
	ID3D11Buffer* variancesCB;

	//Shaders
	ID3D11VertexShader* vertexShader;
	ID3D11GeometryShader* geometryShader;
	ID3D11PixelShader* pixelShader;	
	
	ID3D11ComputeShader* initialiseDeadListCS;
	ID3D11ComputeShader* emitParticlesCS;
	ID3D11ComputeShader* updateParticlesCS;

	//Matrices
	DirectX::XMMATRIX g_World;
	DirectX::XMMATRIX g_View;
	DirectX::XMMATRIX g_Projection;

	DirectX::XMFLOAT4X4 m_World;
	DirectX::XMFLOAT4X4 m_View;
	DirectX::XMFLOAT4X4 m_Projection;


	//Basic Function Requirements
	void Update(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float dt);
	void Draw(ID3D11DeviceContext* deviceContext);

	//Initialises all buffers and compiles shaders etc.
	void InitialiseSystem(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	//MAIN FUNCTIONS FOR SIMULATION
	void InitialiseDeadList(ID3D11DeviceContext* deviceContext);
	void EmitParticles(ID3D11DeviceContext* deviceContext, float dt);
	void UpdateParticles(ID3D11DeviceContext* deviceContext, float dt);

	//READS ATOMIC COUNTER BACK ON CPU, DO NOT USE IN RELEASE CODE
	int ReadCounter(ID3D11UnorderedAccessView* uav);
	ID3D11Buffer* debugCounterBuffer;

	//Buffers, UAVs, SRVs
	ID3D11Buffer* particleDeadListBuffer;
	ID3D11UnorderedAccessView* particleDeadListUAV;

	ID3D11Buffer* particles;
	ID3D11ShaderResourceView* particlesSRV;
	ID3D11UnorderedAccessView* particlesUAV;

	ID3D11Buffer* particleDeadListConstantBuffer;

	////UINT particleCount;
	int particleCounter;

	//Variables and functions for random values
	void FillRandomTexture(ID3D11Device*);
	ID3D11Texture2D* randomValues;
	ID3D11ShaderResourceView* randomValuesSRV;
	ID3D11Texture2D* piRandomValues;
	ID3D11ShaderResourceView* piRandomValuesSRV;
	float elapsedTime;

	//void ConstructShader();
	//std::string shaderCode;

	ID3D11ShaderResourceView* texture;

	void addEmitter(int emitterType, DirectX::XMFLOAT3 emitterPosition, DirectX::XMFLOAT3 emitterVelocity,
		int numToEmitPerFrame, DirectX::XMFLOAT3 particlePosition, DirectX::XMFLOAT3 particleVelocity, float particleLife,
		float startSize, float endSize, DirectX::XMFLOAT4 startColour, DirectX::XMFLOAT4 endColour, DirectX::XMFLOAT3 positionVariance, DirectX::XMFLOAT3 velocityVariance);

	ShaderFactory* shaderFactory;

	//Camera rotation variable
	float rot;

	//Holds data for the desired emitters
	std::vector<EmitterParameters> emittersParams;
	std::vector<Variances> emittersVars;


	std::vector<DirectX::XMFLOAT3> tempEmitterPositions;
};

#endif