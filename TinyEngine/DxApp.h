#pragma once

#include "DxHelper.h"

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
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct SceneConstantBuffer
	{
		XMFLOAT4X4 worldViewProjMatrix;
		float padding[48]; // Padding so the constant buffer is 256-byte aligned.
	};
	static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	bool useWarpDevice;
	float aspectRatio;
	std::wstring assetsPath;

	//指向实例的指针
	static DxApp* app;

	//window
	HWND mhMainWnd = nullptr;
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
	UINT rtvDescriptorSize;

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissorRect;

	ComPtr<ID3D12Resource> constantBuffer;
	SceneConstantBuffer constantBufferData;
	UINT8* pCbvDataBegin;

	UINT frameIndex;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMMATRIX worldViewProj;
	std::vector<Vertex> geometryVertices;
	std::vector<std::uint16_t> geometryIndices;



private:
	std::wstring GetAssetFullPath(LPCWSTR assetName);

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
	void CreateRtvAndCbv();
	void CreateRootSignature();
	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateConstantBuffer();
	void CreateSynObject();
	void WaitForPreviousFrame();
	void PopulateCommandList();

	void OnRender();
	void OnUpdate();
	void OnDestroy();

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
	}

	//给外部调用的接口
	void Init();
	int Run();
};

