#include "GPUParticleSystem.h"
#include <d3dcompiler.h>
#include <string>

GPUParticleSystem::GPUParticleSystem(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	deviceContext = _deviceContext;

	g_maxParticles = 1024 * 1024;

	// Initialize the world matrix
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4(&m_World, World);

	//Initialize the view matrix
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, 0.0f, -20.0f, 0.0f);
	DirectX::XMVECTOR At =	DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, At, Up);
	DirectX::XMStoreFloat4x4(&m_View, View);

	// Initialize the projection matrix
	DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 1920.0f / 1080.0f, 0.00f, 10000.0f);
	DirectX::XMStoreFloat4x4(&m_Projection, Projection);

	particleCounter = 0;

	elapsedTime = 0.0f;
	
	rot = 0;

	//Creates new shader factory for constructing the update shader
	shaderFactory = new ShaderFactory();

	//InitialiseSystem(_device, _deviceContext);
	FillRandomTexture(_device);
}

GPUParticleSystem::~GPUParticleSystem()
{
	mConstBuffer->Release();
	perFrameVariablesBuffer->Release();
	emitterVariablesBuffer->Release();
	variancesCB->Release();
	vertexShader->Release();
	geometryShader->Release();
	pixelShader->Release();
	initialiseDeadListCS->Release();
	emitParticlesCS->Release();
	updateParticlesCS->Release();
	particleDeadListBuffer->Release();
	particleDeadListUAV->Release();
	particles->Release();
	particlesSRV->Release();
	particlesUAV->Release();
	particleDeadListConstantBuffer->Release();
	randomValues->Release();
	piRandomValues->Release();
	randomValuesSRV->Release();
	piRandomValues->Release();
	texture->Release();
	debugCounterBuffer->Release();

	delete shaderFactory;
}

//Helper function for compiling shaders taken from DirectX Tutorials
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			std::string error = std::string((char*)pErrorBlob->GetBufferPointer());
			//core::Debug::log(core::ELL_ERROR, "geometry shader", error);
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

//Helper for compiling compute shaders taken from DirectX documentation
HRESULT CompileComputeShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint,
	_In_ ID3D11Device* device, _Outptr_ ID3DBlob** blob)
{
	if (!srcFile || !entryPoint || !device || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}

void GPUParticleSystem::InitialiseSystem(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	HRESULT hr;

	//Compile the shaders
	ID3DBlob* pVertexShaderBuffer = NULL;
	hr = CompileShaderFromFile(L"VertexShader.hlsl", "VS", "vs_4_0", &pVertexShaderBuffer);

	ID3DBlob* pGeometryShaderBuffer = NULL;
	hr = CompileShaderFromFile(L"VertexShader.hlsl", "GS", "gs_4_0", &pGeometryShaderBuffer);

	ID3DBlob* pPixelShaderBuffer = NULL;
	hr = CompileShaderFromFile(L"VertexShader.hlsl", "PS", "ps_4_0", &pPixelShaderBuffer);

	// Create the shaders
	device->CreateVertexShader(pVertexShaderBuffer->GetBufferPointer(), pVertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);

	device->CreateGeometryShader(pGeometryShaderBuffer->GetBufferPointer(), pGeometryShaderBuffer->GetBufferSize(), NULL, &geometryShader);

	device->CreatePixelShader(pPixelShaderBuffer->GetBufferPointer(), pPixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);

	pVertexShaderBuffer->Release();
	pPixelShaderBuffer->Release();

	//Create buffer constant buffer for draw calls
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MatricesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	
	device->CreateBuffer(&bd, NULL, &mConstBuffer);

	//Create constant buffer for per frame variables
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PerFrameVariables);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	device->CreateBuffer(&bd, NULL, &perFrameVariablesBuffer);

	//Create constant buffer for emitter variables
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(EmitterParameters);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&bd, NULL, &emitterVariablesBuffer);

	//Create constant buffer for variances
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(EmitterParameters);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&bd, NULL, &variancesCB);
	
	//Compile and create compute shaders
	ID3DBlob *csInitBlob = nullptr;
	ID3DBlob *csEmitBlob = nullptr;
	ID3DBlob *csUpdateBlob = nullptr;

	ID3DBlob *errorBlob;

	hr = CompileComputeShader(L"InitialiseDeadList.hlsl", "main", device, &csInitBlob);
	hr = CompileComputeShader(L"EmitParticles.hlsl", "main", device, &csEmitBlob);	
	//hr = CompileComputeShader(L"UpdateParticles.hlsl", "main", device, &csUpdateBlob);

	//Assembles shader code for UpdateParticles
	shaderFactory->ShaderAssembly();

	//Compiles the code that has been put together in the shader factory
	hr = D3DCompile(shaderFactory->updateShaderCode.c_str(), shaderFactory->updateShaderCode.length(), NULL, NULL, NULL,
		"main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG, NULL, &csUpdateBlob, &errorBlob);
	if (FAILED(hr))
	{
		std::string error = std::string((char*)errorBlob->GetBufferPointer());
		errorBlob->Release();

		return;
	}
	
	//Creates the compute shaders
	hr = device->CreateComputeShader(csInitBlob->GetBufferPointer(), csInitBlob->GetBufferSize(), nullptr, &initialiseDeadListCS);
	
	hr = device->CreateComputeShader(csEmitBlob->GetBufferPointer(), csEmitBlob->GetBufferSize(), nullptr, &emitParticlesCS);

	hr = device->CreateComputeShader(csUpdateBlob->GetBufferPointer(), csUpdateBlob->GetBufferSize(), nullptr, &updateParticlesCS);

	// The dead particle index list. Created as an append buffer
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(UINT) * g_maxParticles;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(UINT);

	device->CreateBuffer(&desc, nullptr, &particleDeadListBuffer);

	// Create constant buffers to copy the dead and alive list counters into
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.ByteWidth = 4 * sizeof(UINT);
	device->CreateBuffer(&desc, nullptr, &particleDeadListConstantBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav;
	ZeroMemory(&uav, sizeof(uav));
	uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	uav.Buffer.FirstElement = 0;
	uav.Format = DXGI_FORMAT_UNKNOWN;
	uav.Buffer.NumElements = g_maxParticles;

	device->CreateUnorderedAccessView(particleDeadListBuffer, &uav, &particleDeadListUAV);

	//Initialise list of particles that are able to be emitted
	InitialiseDeadList(deviceContext);

	//Create the particle buffer/UAV/SRV
	D3D11_BUFFER_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(buffDesc));
	buffDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	buffDesc.ByteWidth = sizeof(Particle)* g_maxParticles;
	buffDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buffDesc.StructureByteStride = sizeof(Particle);
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	
	Particle* particleList = new Particle[g_maxParticles];

	//Initial particle values that are never used and are just for initialisation
	for (int i = 0; i < g_maxParticles; ++i)
	{
		particleList[i].position = DirectX::XMFLOAT3(9999.0f, 0.0f, 9999.0f);
		particleList[i].velocity = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		particleList[i].life = 0.0f;
		particleList[i].age = 0.0f;
		particleList[i].alive = false;
		particleList[i].startSize = 0.0f;
		particleList[i].endSize = 0.0f;
		particleList[i].startColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		particleList[i].endColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		particleList[i].currentColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		particleList[i].currentSize = 0.0f;
	}

	D3D11_SUBRESOURCE_DATA	data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = particleList;

	device->CreateBuffer(&buffDesc, &data, &particles);

	delete[] particleList;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.NumElements = g_maxParticles;

	device->CreateShaderResourceView(particles, &srvDesc, &particlesSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.NumElements = g_maxParticles;

	device->CreateUnorderedAccessView(particles, &uavDesc, &particlesUAV);

	//Buffer for the particle count
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.ByteWidth = sizeof(UINT);
	device->CreateBuffer(&desc, nullptr, &debugCounterBuffer);

	//Loads in smoke particle texture to use with the particles using DirectXTK texture loader
	hr = DirectX::CreateWICTextureFromFile(device, deviceContext, L"smoke.jpg", nullptr, &texture);
}

void GPUParticleSystem::addEmitter(int emitterType, DirectX::XMFLOAT3 emitterPosition, DirectX::XMFLOAT3 emitterVelocity,
	int numToEmitPerFrame, DirectX::XMFLOAT3 particlePosition, DirectX::XMFLOAT3 particleVelocity, float particleLife,
	float startSize, float endSize, DirectX::XMFLOAT4 startColour, DirectX::XMFLOAT4 endColour, DirectX::XMFLOAT3 positionVariance, DirectX::XMFLOAT3 velocityVariance)
{
	EmitterParameters emitterParams;
	Variances emitterVariances;

	//Emitter variables
	emitterParams.emitterType = emitterType;
	emitterParams.emitterPosition = emitterPosition;
	emitterParams.emitterVelocity = emitterVelocity;
	
	//Limit maximum number of particles to emit per frame
	if (numToEmitPerFrame > 1024)
	{
		numToEmitPerFrame = 1024;
	}
	emitterParams.numToEmitPerFrame = numToEmitPerFrame;
	
	//Particle variables
	emitterParams.particlePosition = particlePosition;
	emitterParams.particleVelocity = particleVelocity;
	emitterParams.particleLife = particleLife;
	emitterParams.startSize = startSize;
	emitterParams.endSize = endSize;
	emitterParams.startColour = startColour;
	emitterParams.endColour = endColour;
	emitterParams.currentColour = emitterParams.startColour;
	emitterParams.currentSize = emitterParams.startSize;

	//Padding variables to align on the GPU correctly
	emitterParams.pad = 0;
	emitterParams.pad1 = 0;
	emitterParams.pad2 = 0;
	emitterParams.pad3 = 0;
	emitterParams.pad4 = 0;
	emitterParams.pad5 = 0;
	emitterParams.pad6 = 0;
	emitterParams.pad8 = DirectX::XMFLOAT3(0, 0, 0);

	//Variances of the particles
	emitterVariances.positionVariance = positionVariance;
	emitterVariances.velocityVariance = velocityVariance;
	//emitterVariances.lifeVariance = lifeVariance;

	emitterVariances.pad = 0;
	emitterVariances.pad1 = 0;

	//Allows for updating of the emitter position
	DirectX::XMFLOAT3 tempEmitterPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	tempEmitterPositions.push_back(tempEmitterPosition);

	emittersParams.push_back(emitterParams);
	emittersVars.push_back(emitterVariances);
}

//LEGACY CODE FOR CONSTRUCTING EMITTER SHADER

//void GPUParticleSystem::ConstructShader()
//{
//	std::string baseVariables = 
//		"cbuffer PerFrameVariables : register(b0)\n"
//		"{\n"
//		"	float deltaTime;\n"
//		"	float elapsedTime;\n"
//		"}\n"
//		"cbuffer DeadListCount : register(b1)\n"
//		"{\n"
//		"	uint deadCount;\n"
//		"}\n"
//		"cbuffer EmitterParameters : register(b2)\n"
//		"{\n"
//		"	int emitterType;\n"
//		"	float3 emitterPosition;\n"
//		"	int emitterVelocity;\n"
//		"	int positionVariance;\n"
//		"	int velocityVariance;\n"
//		"	int numToEmitPerFrame;\n"
//		"}\n"
//		"struct particles\n"
//		"{\n"
//		"	float3 position;\n"
//		"	float3 velocity;\n"
//		"	float life;\n"
//		"	float age;\n"
//		"	bool alive;\n"
//		"	float startSize;\n"
//		"	float endSize;\n"
//		"	float4 startColour;\n"
//		"	float4 endColour;\n"
//		"};\n";
//	
//	std::string baseStart =
//		"RWStructuredBuffer<particles>	particleBuffer		: register(u0);\n"
//		"ConsumeStructuredBuffer<uint>			deadListToGetFrom	: register(u1);\n"
//		"Texture2D								g_RandomBuffer : register(t0);\n"
//		"SamplerState g_samWrapLinear : register(s0);\n"
//		"[numthreads(1024, 1, 1)]\n"
//		"void main(uint3 DTid : SV_DispatchThreadID)\n"
//		"{\n"
//			"if (DTid.x < deadCount && DTid.x < numToEmitPerFrame)\n"
//			"{\n"
//				"particles p = (particles)0;\n";
//
//	std::string wang =
//		"uint wang_hash(uint seed)"
//		"{"
//		"	seed = (seed ^ 61) ^ (seed >> 16);\n"
//		"	seed *= 9;\n"
//		"	seed = seed ^ (seed >> 4);\n"
//		"	seed *= 0x27d4eb2d;\n"
//		"	seed = seed ^ (seed >> 15);\n"
//		"	return seed;\n"
//		"}\n";
//
//	std::string randomValues =
//		"float2 uv = float2(DTid.x / 1024.0, elapsedTime);\n"
//		"float3 randomValues0 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv, 0).xyz;\n"
//		"float2 uv2 = float2((DTid.x + 1) / 1024.0, elapsedTime);\n"
//		"float3 randomValues1 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv2, 0).xyz;\n"
//		"float2 uv3 = float2((DTid.x - 1) / 1024.0, elapsedTime);\n"
//		"float4 randomValues2 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv3, 0).xyzw;\n"
//		"if(randomValues2.w < 0)\n"
//		"{\n"
//		"randomValues2.w = (randomValues2.w + (2 * randomValues2.w));\n"
//		"}\n"
//		"float2 uv4 = float2((DTid.x - 2) / 1024.0, elapsedTime);\n"
//		"float4 randomValues3 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv4, 0).xyzw;\n";
//
//	//THE PART THAT CHANGES
//		std::string omniEmitter =
//		"p.position.xyz = emitterPosition;\n"
//		"p.velocity.xyz = randomValues1;\n"
//		"p.velocity.y = p.velocity.y + 0.2f;\n"
//		"p.life = randomValues2;\n"
//		"p.age = p.life;\n"
//		"p.alive = true;\n"
//		"p.startSize = 0.0f;\n"
//		"p.endSize = 1.0f;\n"
//		"p.startColour = float4(1.0f, 1.0f, 0.0f, 1.0f);\n"
//		"p.endColour = float4(1.0f, 0.0f, 0.0f, 0.0f);\n";
//
//	std::string volumeCubeEmitter =
//		"p.position.xyz = randomValues1;\n"
//		"p.velocity.xyz = randomValues0;\n"
//		"p.life = randomValues2;\n"
//		"p.age = p.life;\n"
//		"p.alive = true;\n"
//		"p.startSize = 1.0f;\n"
//		"p.endSize = 1.0f;\n"
//		"p.startColour = float4(1.0f, 1.0f, 0.0f, 1.0f);\n"
//		"p.endColour = float4(1.0f, 0.0f, 0.0f, 1.0f);\n";
//
//	std::string volumeTorusEmitter =
//		"float x = emitterPosition.x + (0.3f + (0.1f * cos(randomValues2.w))) * cos(randomValues3.w);\n"
//		"float y = emitterPosition.y + (0.3f + (0.1f * sin(randomValues2.w))) * sin(randomValues3.w);\n"
//		"float z = emitterPosition.z + 0.1f * sin(randomValues2.w);\n"
//
//		"p.position.xyz = float3(x,y,z);\n"
//		"p.velocity.xyz = 0.0f;\n"
//		"p.life = randomValues2;\n"
//		"p.age = p.life;\n"
//		"p.alive = true;\n"
//		"p.startSize = 1.0f;\n"
//		"p.endSize = 1.0f;\n"
//		"p.startColour = float4(1.0f, 1.0f, 0.0f, 1.0f);\n"
//		"p.endColour = float4(1.0f, 0.0f, 0.0f, 1.0f);\n";
//
//	std::string volumeSphereEmitter =
//		"float x = emitterPosition.x + (0.4f * cos(randomValues3.w) * cos(randomValues2.w));\n"
//		"float y = emitterPosition.y + (0.4f * sin(randomValues2.w));\n"
//		"float z = emitterPosition.z + (0.4f * sin(randomValues3.w) * cos(randomValues2.w));\n"
//
//		"p.position.xyz = float3(x,y,z);\n"
//		"p.velocity.xyz = randomValues0;\n"
//		"p.life = randomValues2.w;\n"
//		"p.age = p.life;\n"
//		"p.alive = true;\n"
//		"p.startSize = 1.0f;\n"
//		"p.endSize = 1.0f;\n"
//		"p.startColour = float4(1.0f, 1.0f, 0.0f, 1.0f);\n"
//		"p.endColour = float4(1.0f, 0.0f, 0.0f, 1.0f);\n";
//
//	std::string directionalEmitter =
//		"float x = cos(randomValues0)*sin(randomValues1);\n"
//		"float y = sin(randomValues1)*sin(randomValues0);\n"
//		"float z = cos(randomValues0);\n"
//		
//		"p.position.xyz = emitterPosition;\n"
//		"p.velocity.xyz = float3(x,y,z);\n"
//		"p.life = randomValues2;\n"
//		"p.age = p.life;\n"
//		"p.alive = true;\n"
//		"p.startSize = 1.0f;\n"
//		"p.endSize = 1.0f;\n"
//		"p.startColour = float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
//		"p.endColour = float4(1.0f, 1.0f, 1.0f, 1.0f);\n";
//	////////////////////////////////////////////////////////////////////
//
//	std::string baseEnd =	
//		"uint index = deadListToGetFrom.Consume();\n"
//		"particleBuffer[index] = p;\n"
//		"}\n"
//		"}\n";
//
//	shaderCode = baseVariables + baseStart + randomValues + volumeSphereEmitter + baseEnd;
//}

//Initialises index list of the max number of particles
void GPUParticleSystem::InitialiseDeadList(ID3D11DeviceContext* deviceContext)
{
	deviceContext->CSSetShader(initialiseDeadListCS, nullptr, 0);

	UINT initialCount[] = { 0 };
	deviceContext->CSSetUnorderedAccessViews(0, 1, &particleDeadListUAV, initialCount);

	//Helper for dispatching the right number of thread groups
	int remainder = g_maxParticles % 256;
	int threadGroups = 0;
	if (threadGroups >= 0)
	{
		threadGroups = (g_maxParticles + remainder) / 256;
	}
	// Dispatch a set of 1d thread groups to fill out the dead list, one thread per particle
	deviceContext->Dispatch(threadGroups, 1, 1);

	//Unbinds UAVs
	ID3D11UnorderedAccessView* nullUAV = NULL;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
}

void GPUParticleSystem::EmitParticles(ID3D11DeviceContext* deviceContext, float dt)
{
	//Set UAVs
	ID3D11UnorderedAccessView* uavs[] = { particlesUAV, particleDeadListUAV };
	UINT initialCounts[] = { (UINT)-1, (UINT)-1 };
	deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

	ID3D11ShaderResourceView* srvs[] = { randomValuesSRV, piRandomValuesSRV };
	deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

	//Set compute shader
	deviceContext->CSSetShader(emitParticlesCS, nullptr, 0);
	
	ID3D11Buffer* cBuffers[] = { perFrameVariablesBuffer, particleDeadListConstantBuffer, emitterVariablesBuffer, variancesCB };
	deviceContext->CSSetConstantBuffers(0, 4, cBuffers);

	//Set per frame variables such as dt
	PerFrameVariables PF;
	PF.deltaTime = 0.0f;
	PF.elapsedTime = elapsedTime;
	deviceContext->UpdateSubresource(perFrameVariablesBuffer, 0, NULL, &PF, 0, 0);

	//Passes emitter variables over for each emitter
	for (int i = 0; i < emittersParams.size(); ++i)
	{
		// Update the emitter constant buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		deviceContext->Map(emitterVariablesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		EmitterParameters* EP = (EmitterParameters*)MappedResource.pData;
		EP->emitterType = emittersParams[i].emitterType;
		
		//Temporary solution for updating emitter position
		tempEmitterPositions[i].x += emittersParams[i].emitterVelocity.x;
		tempEmitterPositions[i].y += emittersParams[i].emitterVelocity.y;
		tempEmitterPositions[i].z += emittersParams[i].emitterVelocity.z;

		EP->emitterPosition.x = emittersParams[i].emitterPosition.x + tempEmitterPositions[i].x;
		EP->emitterPosition.y = emittersParams[i].emitterPosition.y + tempEmitterPositions[i].y;
		EP->emitterPosition.z = emittersParams[i].emitterPosition.z + tempEmitterPositions[i].z;

		EP->emitterVelocity = emittersParams[i].emitterVelocity;
		EP->numToEmitPerFrame = emittersParams[i].numToEmitPerFrame;
		EP->particlePosition = emittersParams[i].particlePosition;
		EP->particleVelocity = emittersParams[i].particleVelocity;
		EP->particleLife = emittersParams[i].particleLife;
		EP->startSize = emittersParams[i].startSize;
		EP->endSize = emittersParams[i].endSize;
		EP->startColour = emittersParams[i].startColour;
		EP->endColour = emittersParams[i].endColour;
		EP->currentColour = emittersParams[i].currentColour;
		EP->currentSize = emittersParams[i].currentSize;

		//Padding numbers so that it is aligned when passed to the gpu (potentially could be used)
		EP->pad = emittersParams[i].pad;
		EP->pad1 = emittersParams[i].pad1;
		EP->pad2 = emittersParams[i].pad2;
		EP->pad3 = emittersParams[i].pad3;
		EP->pad4 = emittersParams[i].pad4;
		EP->pad5 = emittersParams[i].pad5;
		EP->pad6 = emittersParams[i].pad6;
		EP->pad8 = emittersParams[i].pad8;
		deviceContext->Unmap(emitterVariablesBuffer, 0);

		//Map variances to generate numbers in random range
		deviceContext->Map(variancesCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		Variances* V = (Variances*)MappedResource.pData;
		V->positionVariance = emittersVars[i].positionVariance;
		V->velocityVariance = emittersVars[i].velocityVariance;
		//V->lifeVariance = emittersVars[i].lifeVariance;

		V->pad = emittersVars[i].pad;
		V->pad1 = emittersVars[i].pad1;
		deviceContext->Unmap(variancesCB, 0);

		deviceContext->CopyStructureCount(particleDeadListConstantBuffer, 0, particleDeadListUAV);

		//Number of particles to emit per frame
		deviceContext->Dispatch(2, 1, 1);
	}

	//Unbind from CS shader so it can be used as a shader resource
	ID3D11UnorderedAccessView* nullUAV = NULL;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);

	//counter to show number of particles
	int counter = ReadCounter(particleDeadListUAV);
	particleCounter = counter;
}


void GPUParticleSystem::UpdateParticles(ID3D11DeviceContext* deviceContext, float dt)
{
	//Set UAVs
	ID3D11UnorderedAccessView* uavs[] = { particlesUAV, particleDeadListUAV };
	UINT initialCounts[] = { (UINT)-1, (UINT)-1 };
	deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

	//Set per frame variables such as dt
	PerFrameVariables PFV;
	PFV.deltaTime = dt;
	deviceContext->UpdateSubresource(perFrameVariablesBuffer, 0, NULL, &PFV, 0, 0);
	
	ID3D11Buffer* cBuffers[] = { perFrameVariablesBuffer, emitterVariablesBuffer };
	deviceContext->CSSetConstantBuffers(0, 2, cBuffers);

	//Set compute shader
	deviceContext->CSSetShader(updateParticlesCS, nullptr, 0);
	
	//Helper for dispatching the right number of thread groups
	int remainder = g_maxParticles % 256;
	int threadGroups = 0;
	if (threadGroups >= 0)
	{
		threadGroups = (g_maxParticles + remainder) / 256;
	}
	deviceContext->Dispatch(threadGroups, 1, 1);

	//Unbind UAVs for compute shader
	ID3D11UnorderedAccessView* nullUAV = NULL;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
}

//Counter so that the number of particles can be read from the buffer
int	GPUParticleSystem::ReadCounter(ID3D11UnorderedAccessView* uav)
{
	int count = 0;

	//Copy the UAV counter to a staging resource
	deviceContext->CopyStructureCount(debugCounterBuffer, 0, uav);

	//Map the staging resource
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	deviceContext->Map(debugCounterBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);

	//Read the data
	count = *(int*)MappedResource.pData;

	deviceContext->Unmap(debugCounterBuffer, 0);

	return count;
}

//Main loop of the system
void GPUParticleSystem::Update(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float dt)
{
	EmitParticles(deviceContext, dt);

	UpdateParticles(deviceContext, dt);

	elapsedTime += dt;

	//Timer that allows the system to select numbers from the random texture
	const float timer = 1.0f;
	if (elapsedTime > timer)
	{
		elapsedTime -= timer;
	}

	//Handles the rotation of the camera
	DirectX::XMMATRIX matRot;
	DirectX::XMMATRIX matTrans;
	matRot = DirectX::XMMatrixRotationY(rot);
	matTrans = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX matFinal = matTrans * matRot;

	DirectX::XMStoreFloat4x4(&m_World, matFinal);
}

void GPUParticleSystem::Draw(ID3D11DeviceContext* deviceContext)
{
	//Sets the matrices
	MatricesConstantBuffer MCB;
	MCB.world = XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_World));
	MCB.view = XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_View));
	MCB.proj = XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_Projection));

	//Pass over constant buffers
	deviceContext->UpdateSubresource(mConstBuffer, 0, NULL, &MCB, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &mConstBuffer);
	deviceContext->GSSetConstantBuffers(0, 1, &mConstBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &mConstBuffer);

	//Null input layout as no vertex buffer is being used
	deviceContext->IASetInputLayout(NULL);

	//Set the shaders and resources
	deviceContext->VSSetShader(vertexShader, 0, 0);
	deviceContext->GSSetShader(geometryShader, 0, 0);
	deviceContext->PSSetShader(pixelShader, 0, 0);

	deviceContext->VSSetShaderResources(0, 1, &particlesSRV);
	deviceContext->PSSetShaderResources(0, 1, &texture);

	//Particles are still points as they are expanded in the geometry shader
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	deviceContext->Draw(g_maxParticles, 0);

	//Unbind shaders and resources so that they can be accessed by other parts of the system if needed
	deviceContext->VSSetShader(NULL, 0, 0);
	deviceContext->GSSetShader(NULL, 0, 0);
	deviceContext->PSSetShader(NULL, 0, 0);

	ID3D11ShaderResourceView* ppSRVNULL[1] = { nullptr };
	deviceContext->VSSetShaderResources(0, 1, ppSRVNULL);
}


float RandomVariance(float median, float variance)
{
	//Gets a number between a positive and negative value 
	float randomValue = (float)rand() / (float)RAND_MAX;
	float range = variance * randomValue;
	return median - variance + (2.0f * range);
}

//Fills the textures with random numbers
void GPUParticleSystem::FillRandomTexture(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = 1024;
	desc.Height = 1024;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	
	const float PI = 3.14159265359;

	//Fills each number within the array with a number in the specified variance
	DirectX::XMFLOAT4* values = new DirectX::XMFLOAT4[desc.Width * desc.Height];
	DirectX::XMFLOAT4* piValues = new DirectX::XMFLOAT4[desc.Width * desc.Height];
	for (UINT i = 0; i < desc.Width * desc.Height; i++)
	{
		values[i].x = RandomVariance(0.0f, 1.0f);
		values[i].y = RandomVariance(0.0f, 1.0f);
		values[i].z = RandomVariance(0.0f, 1.0f);
		values[i].w = RandomVariance(0.0f, 1.0f);

		piValues[i].x = RandomVariance(0.0f, 2.0f * PI);
		piValues[i].y = RandomVariance(0.0f, 2.0f * PI);
		piValues[i].z = RandomVariance(0.0f, 2.0f * PI);
		piValues[i].w = RandomVariance(0.0f, 2.0f * PI);
	}

	//Creates the textures and SRVs to be passed to the GPU
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = values;
	data.SysMemPitch = desc.Width * 16;
	data.SysMemSlicePitch = 0;

	device->CreateTexture2D(&desc, &data, &randomValues);

	data.pSysMem = piValues;
	data.SysMemPitch = desc.Width * 16;
	data.SysMemSlicePitch = 0;

	device->CreateTexture2D(&desc, &data, &piRandomValues);

	delete[] values;
	delete[] piValues;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	srv.Format = desc.Format;
	srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.Texture2D.MipLevels = 1;
	srv.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(randomValues, &srv, &randomValuesSRV);
	device->CreateShaderResourceView(piRandomValues, &srv, &piRandomValuesSRV);
}