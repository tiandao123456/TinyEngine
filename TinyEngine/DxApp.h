#pragma once

#include "DxHelper.h"
#include "SceneManage.h"
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
	static HWND mhMainWnd;

private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};
	
	struct ObjectConstant
	{
		XMFLOAT4X4 worldViewProjMatrix;
	};
	
	bool useWarpDevice;
	float aspectRatio;
	std::wstring assetsPath;

	//指向实例的指针
	static DxApp* app;

	//window
	HINSTANCE mhAppInst = nullptr;
	std::wstring mMainWndCaption = L"d3d App";

	//directx12
	ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
	ComPtr<ID3D12Device> d3dDevice = nullptr;

	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	ComPtr<IDXGISwapChain3> dxgiSwapChain = nullptr;

	ComPtr<ID3D12Resource> renderTargets[swapChainBufferCount];
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	UINT rtvDescriptorSize;

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	ComPtr<ID3D12Resource> uploadBuffer = nullptr;
	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissorRect;

	ComPtr<ID3D12Resource> constantBuffer;
	std::unique_ptr<UploadHeapConstantBuffer<ObjectConstant>> objectConstantBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHeapHandle;
	ComPtr<ID3D12Resource> depthStencilBuffer;

	UINT frameIndex;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT cbvSrvUavDescriptorSize;

	//存放场景数据的容器
	std::vector<staticMeshActor> staticMeshDatas;
	cameraInfo cameraData;

	std::vector<std::vector<float>> modelMatrixDatas;
	std::vector<int> staticMeshIndicesNums;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	std::vector<Vertex> allVertices;
	std::vector<std::uint16_t> allIndices;
	bool isRunning = true;

private:
	bool messageLoop();
	//使用getApp获取static指针
	static DxApp* getApp()
	{
		return app;
	}
	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool InitMainWindow();
	bool InitDirectx12();
	void EnumAdapter();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndCbvAndDsv();
	void CreateRootSignature();
	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateConstantBuffer();
	void CreateDepthStencil();
	void CreateConstantBufferForCone();
	void CreateSynObject();
	void WaitForPreviousFrame();
	void PopulateCommandList();

	void OnRender();
	void OnUpdate();
	void OnDestroy();
	void GetSceneDatas();
	void CreateDefaultHeapBuffer(ID3D12GraphicsCommandList* cmdList, const void* data, const int size, ComPtr<ID3D12Resource>& vertexBuffer);

public:
	//在构造函数中初始化static指针
	DxApp(HINSTANCE parameter) :mhAppInst(parameter),
		viewport(0.0f, 0.0f, static_cast<float>(mClientWidth), static_cast<float>(mClientHeight)),
		scissorRect(0, 0, static_cast<LONG>(mClientWidth), static_cast<LONG>(mClientHeight))
	{
		app = this;
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

	//给外部调用的接口
	void Init();
	void Run();
};

