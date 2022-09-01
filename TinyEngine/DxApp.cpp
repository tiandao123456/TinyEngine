#include "stdafx.h"
#include "DxApp.h"

DxApp* DxApp::app = nullptr;

void DxApp::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(dxgiSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void DxApp::OnUpdate()
{
	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(-30, 140, 90, 1.0f);
	XMVECTOR target = XMVectorSet(-30.766044439721629, 139.35721238626465, 90, 1.0f);
	XMVECTOR up = XMVectorSet(0.0, 0.0f, 1.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX world = { 1,0,0,0,0,1,0,0,0,0,1,0,-190,20,50,1 };
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Pi, aspectRatio, 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX worldViewProj = world * view * proj;


	XMStoreFloat4x4(&constantBufferData.worldViewProjMatrix, XMMatrixTranspose(worldViewProj));

	memcpy(pCbvDataBegin, &constantBufferData, sizeof(constantBufferData));

}

void DxApp::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));

	// Set necessary state.
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	// Indicate that the back buffer will be used as a render target.
	auto x = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &x);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(
		(UINT)geometryIndices.size(),
		1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	auto y = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &y);

	ThrowIfFailed(commandList->Close());
}

//回调函数,有鼠标或键盘消息就触发
LRESULT CALLBACK DxApp::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		//获取实例指针
		DxApp::getApp()->OnUpdate();
		DxApp::getApp()->OnRender();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//窗口的初始化
bool DxApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);

	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);

	return true;
}

void DxApp::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(fenceEvent);
}

int DxApp::Run()
{

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	OnDestroy();

	return static_cast<char>(msg.wParam);
}

std::wstring DxApp::GetAssetFullPath(LPCWSTR assetName)
{
	WCHAR assetsPathParameter[512];
	GetAssetsPath(assetsPathParameter, _countof(assetsPathParameter));
	assetsPath = assetsPathParameter;
	return assetsPath + assetName;
}

void DxApp::EnumAdapter()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	//创建DXGI工厂，使用DXGI工厂来生产DXGI对象
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	//创建一个表示显示适配器的设备
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&d3dDevice));

	//如果创建硬件显示适配器失败，则枚举软件显示适配器
	//软件适配器一样可以进行渲染管线等一系列操作
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&d3dDevice)));
	}
}

//创建命令队列，命令列表以及命令分配器
//cpu能够将命令列表中的命令提交到命令队列中
//多核cpu能够创建多个命令列表，发挥多核心的优势
//而gpu则从命令队列中取命令执行

//命令分配器
//1、负责将命令列表中的命令提交到命令队列之中
//2、清空命令列表中的命令但是使之保持原本的容量避免频繁的内存分配带来的消耗
void DxApp::CreateCommandObjects()
{
}

void DxApp::CreateSwapChain()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//由显卡适配器创建命令队列
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

	// 描述交换链并创建
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = swapChainBufferCount;
	swapChainDesc.Width = mClientWidth;
	swapChainDesc.Height = mClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		mhMainWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&dxgiSwapChain));
	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

void DxApp::CreateRtvAndCbv()
{
	// descriptor表示资源在显存中的存放地址（该资源可能是顶点纹理等）
	// heaps则是存放descriptor的一片内存
	{
		// 创建渲染目标视图（渲染目标视图是渲染管线的最终目标输出的位置，因为渲染管线最终不会输出到屏幕上，会先输出到渲染目标视图上）
		// 最后屏幕获取渲染目标视图的内容并呈现
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = swapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

		rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));
	}

	// Create frame resources.
	{
		//??
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < swapChainBufferCount; n++)
		{
			ThrowIfFailed(dxgiSwapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
			d3dDevice->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, rtvDescriptorSize);
		}
	}
	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void DxApp::CreateRootSignature()
{
	//创建根签名，该根签名包含一个描述符表，该描述符表中有一个CBV
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];

	//在此处说明了根签名中有CBV
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}


void DxApp::CreatePipelineState()
{
	// Create the pipeline state, which includes compiling and loading shaders.
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(L"Shaders\\color.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"Shaders\\color.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		// 12代表顶点的大小，即x,y,z共计三个字节
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	// Create the command list.
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(commandList->Close());
}

void DxApp::CreateVertexBuffer()
{
	// Create the vertex buffer.
	// Define the geometry for a triangle.

	aspectRatio = static_cast<float>(mClientWidth) / static_cast<float>(mClientHeight);

	//从文件中读取顶点数据
	//std::fstream verticesFile("vertices.txt");
	std::fstream verticesFile("./Datafile/vertices.txt");
	if (!verticesFile)
		exit(-1);
	for (int i = 0; i < 1467; i++)
	{
		double x, y, z;
		char c;
		verticesFile >> x >> c;
		verticesFile >> y >> c;
		verticesFile >> z >> c;

		////windows桌面程序的输出黑窗口
		//AllocConsole();
		//_cprintf("i=%f ", x);
		//_cprintf("i=%f ", y);
		//_cprintf("i=%f\n", z);
		geometryVertices.push_back({ XMFLOAT3(x, y, z),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) });
	}

	//std::fstream indicesFile("indices.txt");
	std::fstream indicesFile("./Datafile/indices.txt");
	if (!indicesFile)
		exit(-1);
	for (int i = 0; i < 1782; i++)
	{
		char c;
		std::uint16_t x, y, z;
		indicesFile >> x >> c;
		indicesFile >> y >> c;
		indicesFile >> z >> c;

		geometryIndices.push_back(x);
		geometryIndices.push_back(y);
		geometryIndices.push_back(z);
	}

	//geometryVertices =
	//{
	//	//顶点的坐标与颜色信息RGBA
	//	Vertex({ XMFLOAT3(-1.0f, -1.0f , -1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(-1.0f, +1.0f , -1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(+1.0f, +1.0f , -1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(+1.0f, -1.0f , -1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(-1.0f, -1.0f , +1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(-1.0f, +1.0f , +1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(+1.0f, +1.0f , +1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) }),
	//	Vertex({ XMFLOAT3(+1.0f, -1.0f , +1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) })
	//};

	//geometryIndices =
	//{
	//	// front face
	//	0, 1, 2,
	//	0, 2, 3,

	//	// back face
	//	4, 6, 5,
	//	4, 7, 6,

	//	// left face
	//	4, 5, 1,
	//	4, 1, 0,

	//	// right face
	//	3, 2, 6,
	//	3, 6, 7,

	//	// top face
	//	1, 5, 6,
	//	1, 6, 2,

	//	// bottom face
	//	4, 0, 3,
	//	4, 3, 7
	//};

	const UINT vbByteSize = (UINT)geometryVertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)geometryIndices.size() * sizeof(std::uint16_t);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	// 创建m_vertexBuffer，该对象既能够访问内存又能够访问显存
	// 使用该对象读取内存中的数据并传输到显存

	auto x1 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto y1 = CD3DX12_RESOURCE_DESC::Buffer(vbByteSize);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&x1,
		D3D12_HEAP_FLAG_NONE,
		&y1,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)));

	// 将内存数据拷贝到显存
	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange1(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(vertexBuffer->Map(0, &readRange1, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, &geometryVertices[0], sizeof(Vertex)*geometryVertices.size());
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vbByteSize;

	auto x2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto y2 = CD3DX12_RESOURCE_DESC::Buffer(ibByteSize);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&x2,
		D3D12_HEAP_FLAG_NONE,
		&y2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)));

	// 将内存数据拷贝到显存
	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange2(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(indexBuffer->Map(0, &readRange2, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, &geometryIndices[0], sizeof(std::uint16_t) * geometryIndices.size());
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the index buffer view.
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vbByteSize;

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = ibByteSize;
}

void DxApp::CreateConstantBuffer()
{
	const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.

	auto x = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto y = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&x,
		D3D12_HEAP_FLAG_NONE,
		&y,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)));

	// Describe and create a constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;
	d3dDevice->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin)));
	memcpy(pCbvDataBegin, &constantBufferData, sizeof(constantBufferData));
}

void DxApp::CreateSynObject()
{
	ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValue = 1;

	// Create an event handle to use for frame synchronization.
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForPreviousFrame();
}

void DxApp::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fenceParameter = fenceValue;
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceParameter));
	fenceValue++;

	// Wait until the previous frame is finished.
	if (fence->GetCompletedValue() < fenceParameter)
	{
		ThrowIfFailed(fence->SetEventOnCompletion(fenceParameter, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

void DxApp::CreateWorldViewProj()
{
	//将模型从本地坐标转换到世界坐标系中
	//包含平移缩放旋转等
	XMMATRIX worldMatrix = { 1,0,0,0,0,1,0,0,0,0,1,0,-190,20,50,1 };

	//将模型从世界坐标系中转换到相机的坐标系中
	XMVECTOR pos = XMVectorSet(-30, 140, 90, 1.0f);
	XMVECTOR target = XMVectorSet(-30.766044439721629, 139.35721238626465, 90, 1.0f);
	XMVECTOR up = XMVectorSet(0.0, 0.0f, 1.0f, 0.0f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, target, up);

	//对模型进行透视投影变换
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(0.25f * Pi, aspectRatio, 1.0f, 1000.0f);

	worldViewProj = worldMatrix * viewMatrix * projMatrix;
}

//初始化dx12
bool DxApp::InitDirectx12()
{
	//枚举适配器
	EnumAdapter();
	CreateSwapChain();
	CreateRtvAndCbv();
	CreateRootSignature();
	CreatePipelineState();
	CreateVertexBuffer();
	CreateConstantBuffer();
	CreateWorldViewProj();
	CreateSynObject();

	return true;
}

void DxApp::Init()
{
	InitMainWindow();
	InitDirectx12();
}




