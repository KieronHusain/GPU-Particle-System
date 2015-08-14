#ifndef __SHADER_FACTORY_H__
#define __SHADER_FACTORY_H__

#include <windows.h>
#include <d3d11_1.h>
#include <string>
#include <DirectXMath.h>
#include <sstream>
#include <vector>

class ShaderFactory
{
public:

	ShaderFactory();
	~ShaderFactory();

	void ShaderAssembly();

	void AddGravityWindField(DirectX::XMFLOAT3 force);
	void AddRadialField(DirectX::XMFLOAT3 position, float radius, float forceOutwards);
	void AddVortexField(DirectX::XMFLOAT3 position, float radius, float height, float speed);

	std::string updateShaderCode;

	std::string Convert(float toConvert);

	std::vector<std::string> listOfStrings;
	std::vector<std::string> gravityFields;
};

#endif