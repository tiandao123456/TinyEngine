#pragma once
#include "RHIBase.h"
#include "Common/DxHelper.h"
#include "JsonFileParse.h"
#include "d3dUtil.h"
#include "Win.h"
#include <unordered_map>
#include <d3d12.h>
#include <array>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3D12RHI :public RHIBase
{
public:
	static const int swapChainBufferCount = 2;
public:
	virtual void RHIStartDebug();
	virtual void RHIEnumAdapter();
	virtual void RHICreateCommandObjects();
	virtual void RHICreateSwapChain(UINT bufferCount, Dataformat dFormat);
	virtual void RHICreateRenderTarget(UINT renderTargetNums);
	virtual void RHICreateDepthStencil(Dataformat dFormat, UINT shadowMapWidth, UINT shadowMapHeight);
	virtual void RHICreateRootDescriptorTable();
	virtual void RHICreateConstBufferAndShaderResource();
	virtual void RHISetVertexAndIndexBuffer();
	virtual void RHICreatePipeLineState();
	virtual void RHICreateSynObject();
	virtual void RHIPopulateCommandList();
	virtual void RHIUpdate();
	virtual void RHIRender();
	virtual void RHIDrawSceneToShadow();

	void LoadTexture();
	void WaitForPreviousFrame();
	void CalculateCameraViewProj();
	void CalculateLightViewProjTex();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 2> GetStaticSamplers();
	void CreateDefaultHeapBuffer(ID3D12GraphicsCommandList* cmdList, const void* data, const int size, ComPtr<ID3D12Resource>& vertexBuffer);

private:
	ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
	ComPtr<ID3D12Device> d3dDevice = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<IDXGISwapChain3> dxgiSwapChain = nullptr;
	UINT frameIndex;
	UINT rtvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	ComPtr<ID3D12Resource> renderTargets[swapChainBufferCount];
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;

private:
	UINT dsvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	ComPtr<ID3D12Resource> depthStencilBuffer = nullptr;
	ComPtr<ID3D12Resource> shadowMapResource = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHeapHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowDsvHeapHandle;
	D3D12_VIEWPORT shadowMapViewport;
	D3D12_RECT shadowMapScissorRect;
	D3D12_VIEWPORT baseViewport = { 0.0f, 0.0f, static_cast<float>(mClientWidth), static_cast<float>(mClientHeight) };
	D3D12_RECT baseScissorRect = { 0, 0, static_cast<LONG>(mClientWidth), static_cast<LONG>(mClientHeight) };

private:
	UINT cbvSrvUavDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> cbvSrvUavHeap = nullptr;
	std::unique_ptr<UploadHeapConstantBuffer<WorldMatrix>> worldMatrixBuffer;
	std::unique_ptr<UploadHeapConstantBuffer<ConstMatrix>> constMatrixBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandle;

private:
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	std::vector<int> staticMeshIndicesNums;
	std::vector<int> staticMeshVerticesNums;
 	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	std::vector<Vertex> allVertices;
	std::vector<std::uint16_t> allIndices;
	ComPtr<ID3D12Resource> uploadBuffer = nullptr;

private:
	std::unordered_map<std::string, ComPtr<ID3DBlob>> shaders;
	ComPtr<ID3D12PipelineState> basePassPipelineState = nullptr;
	ComPtr<ID3D12PipelineState> shadowPassPipelineState = nullptr;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

private:
	UINT cbvNums = 0;
	UINT srvNums = 0;
	XMMATRIX cameraViewProjMatrix;
	XMMATRIX lightViewProjMatrix;
	XMMATRIX lightViewProjTexMatrix;

	std::vector<int> recordIndexStartPosition;
	std::vector<int> recondVertexStartPosition;
};

