#pragma once
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

//顶点结构：位置、法线、uv坐标
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
};

struct Camera
{
	std::vector<float> location;
	std::vector<float> target;
	float aspect;
};

struct StaticMesh
{
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices;
};

//静态网格体数据
struct StaticMeshActor
{
	StaticMesh staticMesh;
	std::vector<std::vector<float>> worldMatrix;
	int indiceNum;
};

struct Light
{
	std::vector<float> location;
	std::vector<float> direction;
	float intensity;
};

struct WorldMatrix
{
	XMFLOAT4X4 worldMatrix;
};

struct ConstMatrix
{
	XMFLOAT4X4 cameraVPMatrix;
	XMFLOAT4X4 lightVPMatrix;
	XMFLOAT4X4 lightVPTexMatrix;
	XMFLOAT3 cameraLoc;
};

struct Texture
{
	std::string name;
	std::wstring filename;
	ComPtr<ID3D12Resource> resource = nullptr;
	ComPtr<ID3D12Resource> uploadHeap = nullptr;
};