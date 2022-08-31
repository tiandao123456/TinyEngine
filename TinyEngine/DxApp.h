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
	UINT rtvDescriptorSize;

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissorRect;

	UINT frameIndex;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

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
	void CreateRtv();
	void CreateRootSignature();
	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateSynObject();
	void WaitForPreviousFrame();
	void PopulateCommandList();

	void OnRender();
	void OnDestroy();

public:
	//在构造函数中初始化static指针
	DxApp(HINSTANCE parameter) :mhAppInst(parameter),
		viewport(0.0f, 0.0f, static_cast<float>(mClientWidth), static_cast<float>(mClientHeight)),
		scissorRect(0, 0, static_cast<LONG>(mClientWidth), static_cast<LONG>(mClientHeight))
	{
		app = this;
	}

	//给外部调用的接口
	void Init();
	int Run();
};

