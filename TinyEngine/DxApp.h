#pragma once

#include "Common/DxHelper.h"
#include "SceneManage.h"
#include "d3dUtil.h"
#include "ShadowMap.h"
#include <unordered_map>
#include <d3d12.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class DxApp
{
public:
	static const int swapChainBufferCount = 2;
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 color;
	};
	
	struct WorldMatrix
	{
		XMFLOAT4X4 worldMatrix;
	};

	struct ConstMatrix
	{
		XMFLOAT4X4 viewProjMatrix;
		XMFLOAT4X4 shadowTransform;
		XMFLOAT4X4 shadowMatirx;
	};

	struct MaterialConstant
	{
		// 散射光反射率
		DirectX::XMFLOAT4 diffuseAlbedo;
		// 菲涅耳R0参数
		DirectX::XMFLOAT3 fresnelR0;
		// 粗糙度
		float roughness;
	};
	
	bool useWarpDevice;
	float aspectRatio;
	//static DxApp *app;

	//directx12
	ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
	ComPtr<ID3D12Device> d3dDevice = nullptr;

	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;
	ComPtr<ID3D12PipelineState> shadowPipelineState = nullptr;

	ComPtr<IDXGISwapChain3> dxgiSwapChain = nullptr;

	ComPtr<ID3D12Resource> renderTargets[swapChainBufferCount];
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> cbvSrvUavHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	UINT rtvDescriptorSize;

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	ComPtr<ID3D12Resource> uploadBuffer = nullptr;
	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissorRect;

	std::unique_ptr<UploadHeapConstantBuffer<WorldMatrix>> worldMatrixConstantBuffer;
	std::unique_ptr<UploadHeapConstantBuffer<ConstMatrix>> ConstantBuffer;
	std::unique_ptr<UploadHeapConstantBuffer<MaterialConstant>> materialConstantBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandle;
	ComPtr<ID3D12Resource> depthStencilBuffer;

	UINT frameIndex;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT cbvSrvUavDescriptorSize;
	std::vector<int> staticMeshIndicesNums;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	std::vector<Vertex> allVertices;
	std::vector<std::uint16_t> allIndices;
	bool isRunning = true;

	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
	XMMATRIX viewProjMatrixParam;
	XMMATRIX shadowMatrixParam;

	std::unique_ptr<ShadowMap> shadowMap;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> shaders;
private:
	////使用getApp获取static指针
	//static DxApp* getApp()
	//{
	//	return app;
	//}
	void EnumAdapter();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeap();
	void LoadTexture();
	void CreateRootSignature();
	void CreatePipelineState();
	void CreateVertexAndIndexBuffer();
	void CreateCbvSrvUavDescriptor();
	void CreateDepthStencil();
	void CreateSynObject();
	void WaitForPreviousFrame();
	void PopulateCommandList();
	void CalculateViewProj();
	void CreateShadowMap();
	void CalculateShadowTransform();
	void DrawSceneToShadow();

	void OnRender();
	void OnUpdate();
	void OnDestroy();
	void CreateDefaultHeapBuffer(ID3D12GraphicsCommandList* cmdList, const void* data, const int size, ComPtr<ID3D12Resource>& vertexBuffer);

public:
	//在构造函数中初始化static指针
	DxApp():
		viewport(0.0f, 0.0f, static_cast<float>(mClientWidth), static_cast<float>(mClientHeight)),
		scissorRect(0, 0, static_cast<LONG>(mClientWidth), static_cast<LONG>(mClientHeight))
	{
		//app = this;
		mWorld = { 1.0f,0.0f,0.0f,0.0f,
				   0.0f,1.0f,0.0f,0.0f,
				   0.0f,0.0f,1.0f,0.0f,
				   0.0f,0.0f,0.0f,1.0f };
		mView = { 1.0f,0.0f,0.0f,0.0f,
				   0.0f,1.0f,0.0f,0.0f,
				   0.0f,0.0f,1.0f,0.0f,
				   0.0f,0.0f,0.0f,1.0f };
		mProj = { 1.0f,0.0f,0.0f,0.0f,
				   0.0f,1.0f,0.0f,0.0f,
				   0.0f,0.0f,1.0f,0.0f,
				   0.0f,0.0f,0.0f,1.0f };
		aspectRatio = static_cast<float>(mClientWidth) / static_cast<float>(mClientHeight);
	}
	//外部调用接口
	bool InitDirectx12();
	void RenderTick();
};

