#include "stdafx.h"
#include "DxApp.h"
#include "TEngine.h"

//DxApp* DxApp::app = nullptr;

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

//更新worldViewProj矩阵
void DxApp::OnUpdate()
{
	XMVECTOR pos = XMVectorSet(TEngine::cameraData.location[0], TEngine::cameraData.location[1], TEngine::cameraData.location[2], 1.0f);
	XMVECTOR target = XMVectorSet(TEngine::cameraData.target[0], TEngine::cameraData.target[1], TEngine::cameraData.target[2], 1.0f);
	XMVECTOR up = XMVectorSet(0.0, 0.0f, 1.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	int position = 0;
	for(auto i=0;i<TEngine::modelMatrixDatas.size();i++)
	{
		XMMATRIX world =
		{
			TEngine::modelMatrixDatas[i][0],TEngine::modelMatrixDatas[i][1],TEngine::modelMatrixDatas[i][2],TEngine::modelMatrixDatas[i][3],
			TEngine::modelMatrixDatas[i][4],TEngine::modelMatrixDatas[i][5],TEngine::modelMatrixDatas[i][6],TEngine::modelMatrixDatas[i][7],
			TEngine::modelMatrixDatas[i][8],TEngine::modelMatrixDatas[i][9],TEngine::modelMatrixDatas[i][10],TEngine::modelMatrixDatas[i][11],
			TEngine::modelMatrixDatas[i][12],TEngine::modelMatrixDatas[i][13],TEngine::modelMatrixDatas[i][14],TEngine::modelMatrixDatas[i][15]
		};
		XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * Pi, aspectRatio, 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, p);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);

		XMMATRIX worldViewProj = world * view * proj;

		ObjectConstant objConstants;
		XMStoreFloat4x4(&objConstants.worldViewProjMatrix, XMMatrixTranspose(worldViewProj));
		objectConstantBuffer->CopyData(position++, objConstants);
	}
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

	// 设置根签名，描述符堆以及描述符表
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	//绑定视口到渲染管线的光栅化阶段
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	// 表明后台缓冲将被使用作为渲染目标
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto x = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// 通知驱动程序它需要同步对资源的多次访问 
	// 用资源屏障来完成同步，资源屏障即进行资源的状态转换
	// GPU上某个ALU对该资源可能是只读操作，某个ALU可能是只写操作，使用资源屏障就可以给资源一个状态
	// 当资源变成只读状态时，表明某些ALU可以进行对资源进行操作了，只写同理
	commandList->ResourceBarrier(1, &x);

	ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvUavHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE handleTemp = handle;
	for (auto i = 0; i < TEngine::staticMeshDatas.size(); i++)
		handleTemp.Offset(cbvSrvUavDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(1, handleTemp);

	handleTemp.Offset(cbvSrvUavDescriptorSize);
	for (auto i = 0; i < TEngine::staticMeshDatas.size(); i++)
	{
		handle.Offset(cbvSrvUavDescriptorSize * i);
		//第一个参数表示根签名中的数组下标，0表示为根签名的第一个元素
		//根签名中的第一个元素对应着多个CBV所以需要偏移
		commandList->SetGraphicsRootDescriptorTable(0, handle);
		if (i == 0)
		{
			commandList->SetGraphicsRootDescriptorTable(2, handleTemp);
			// 绘制一个实例，第一个参数为索引数量
			commandList->DrawIndexedInstanced((UINT)staticMeshIndicesNums[0], 1, 0, 0, 0);
		}
		else
		{
			handleTemp.Offset(cbvSrvUavDescriptorSize);
			handleTemp.Offset(cbvSrvUavDescriptorSize);
			commandList->SetGraphicsRootDescriptorTable(2, handleTemp);
			commandList->DrawIndexedInstanced((UINT)staticMeshIndicesNums[i], 1, (UINT)staticMeshIndicesNums[i - 1], TEngine::staticMeshDatas[i - 1].vertices.size(), 0);
		}
	}
	auto y = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	// 表明后台缓冲现在将被呈现
	commandList->ResourceBarrier(1, &y);

	ThrowIfFailed(commandList->Close());
}

void DxApp::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(fenceEvent);
}

void DxApp::EnumAdapter()
{
#if defined(DEBUG) || defined(_DEBUG) 
	{
		ComPtr<ID3D12Debug> debugController;
		//获取调试接口
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		//开启调试功能
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
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//由显卡适配器创建命令队列，命令队列由GPU维护
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	// Create the command list.
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));
}

void DxApp::CreateSwapChain()
{
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
		commandQueue.Get(),        // 交换链需要命令队列才能刷新，比如命令队列中一个指令指示交换链进行前后台缓冲的交换
		TEngine::mhMainWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	//该窗口将不会响应alt-enter序列（按键组合）
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(TEngine::mhMainWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&dxgiSwapChain));
	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

void DxApp::CreateRtvAndDsvDescriptorHeap()
{
	// descriptor表示资源在显存中的存放地址（该资源可能是顶点纹理等）
	// heaps则是存放descriptor的一片内存
	// 创建渲染目标视图（渲染目标视图是渲染管线的最终目标输出的位置，因为渲染管线最终不会输出到屏幕上，会先输出到渲染目标视图上）
	// 最后屏幕获取渲染目标视图的内容并呈现
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = swapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

	// 计算RTV描述符的大小
	rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//获得渲染目标视图描述符堆的起始位置
	//		| RTV Descriptor Heap |
	//		|---------------------|
	//0		| BackBuffer0's rtv   |
	//		|---------------------|
	//1		| BackBuffer1's rtv   |
	//		|---------------------|
	//上面创建了渲染目标视图的描述符堆，之后在该堆的基础上再创建渲染目标视图
	//所以需要先拿到堆的起始地址，之后根据描述符的大小进行偏移创建渲染目标视图
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	// Create a RTV for each frame.
	for (UINT n = 0; n < swapChainBufferCount; n++)
	{
		ThrowIfFailed(dxgiSwapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
		d3dDevice->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize);
	}
	//创建深度模板堆
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));
}

//不开启深度模板缓冲的话，后绘的物体会挡住先绘制的物体，而不是根据实际的深度绘制
void DxApp::CreateDepthStencil()
{
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC tex2D = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mClientWidth, mClientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&tex2D,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)));

	d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DxApp::LoadTexture()
{
	//从文件夹中读取两个dds文件
	auto diffuseTex1 = std::make_unique<Texture>();
	diffuseTex1->Name = "diffuseTex1";
	diffuseTex1->Filename = L"./Resources/bricks.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), diffuseTex1->Filename.c_str(),
		diffuseTex1->Resource, diffuseTex1->UploadHeap));

	auto normalTex1 = std::make_unique<Texture>();
	normalTex1->Name = "normalTex1";
	normalTex1->Filename = L"./Resources/bricks_nmap.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), normalTex1->Filename.c_str(),
		normalTex1->Resource, normalTex1->UploadHeap));

	auto diffuseTex2 = std::make_unique<Texture>();
	diffuseTex2->Name = "diffuseTex2";
	diffuseTex2->Filename = L"./Resources/tile.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), diffuseTex2->Filename.c_str(),
		diffuseTex2->Resource, diffuseTex2->UploadHeap));

	auto normalTex2 = std::make_unique<Texture>();
	normalTex2->Name = "normalTex2";
	normalTex2->Filename = L"./Resources/tile_nmap.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), normalTex2->Filename.c_str(),
		normalTex2->Resource, normalTex2->UploadHeap));

	//键值对保存，name对应uni_ptr
	textures[diffuseTex1->Name] = std::move(diffuseTex1);
	textures[normalTex1->Name] = std::move(normalTex1);
	textures[diffuseTex2->Name] = std::move(diffuseTex2);
	textures[normalTex2->Name] = std::move(normalTex2);
}

//描述符表与描述符堆的关系：描述符表实际上是描述符堆的子范围
//SRV如果想要传进shader中，必须放入到描述符表中（descriptor table）
//然后CBV同样也可以放到描述符表中，也可以不放直接传输到shader中
void DxApp::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

	//有两个texture、一个const buffer
	//是对范围的描述，而constantBufferView并不是范围
	CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
	//绑定两个CBV到寄存器b0上
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	//绑定两个SRV分别到寄存器t0、t1上面
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

	//采样器描述
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	//实时编译着色器
	ThrowIfFailed(D3DCompileFromFile(L"Shaders\\color.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"Shaders\\color.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	//定义顶点的输入布局
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		//顶点位置从0偏移处开始，顶点颜色从第12个字节（x,y,z）开始
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//描述混和状态
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);//深度模板状态
	psoDesc.SampleMask = UINT_MAX;
	//解释集合或外壳着色器输入图元，此处定义图元为三角形
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//描述资源的多重采样，此处设置为1即不进行多重采样
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	// 命令列表创建是用来记录命令的，由于现在没有命令可记录，因此关闭
	// ThrowIfFailed(commandList->Close());
	//由于后面有命令所以不能close
}
void DxApp::CreateDefaultHeapBuffer(ID3D12GraphicsCommandList* cmdList, const void* data, const int size, ComPtr<ID3D12Resource> &vertexBuffer)
{
	//将资源从上传堆拷贝到默认堆
	//下述两种方法都不行，可能是d3dx12文件版本问题
	//不同版本中函数的实现不同
	//方法一
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.

	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto descSize = CD3DX12_RESOURCE_DESC::Buffer(size);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&descSize,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&descSize,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = data;
	subResourceData.RowPitch = size;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	// 转换资源的状态，将资源从upload buffer拷贝到default buffer
	auto barrierFromCommonToDest = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &barrierFromCommonToDest);
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

	//转换资源状态，资源变为可读
	auto barrierFromDestToRead = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &barrierFromDestToRead);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.
	vertexBuffer = defaultBuffer;

	////方法二
	//D3D12_HEAP_PROPERTIES defaultHeap;
	//memset(&defaultHeap, 0, sizeof(defaultHeap));
	//defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

	//D3D12_HEAP_PROPERTIES  uploadHeap;
	//memset(&uploadHeap, 0, sizeof(uploadHeap));
	//uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

	////创建VertexBuffer的资源描述
	//D3D12_RESOURCE_DESC defaultBufferDesc;
	//memset(&defaultBufferDesc, 0, sizeof(D3D12_RESOURCE_DESC));
	//defaultBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//defaultBufferDesc.Alignment = 0;
	//defaultBufferDesc.Width = size;
	//defaultBufferDesc.Height = 1;
	//defaultBufferDesc.DepthOrArraySize = 1;
	//defaultBufferDesc.MipLevels = 1;
	//defaultBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	//defaultBufferDesc.SampleDesc.Count = 1;
	//defaultBufferDesc.SampleDesc.Quality = 0;
	//defaultBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//defaultBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	//// Note: using upload heaps to transfer static data like vert buffers is not 
	//// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//// over. Please read up on Default Heap usage. An upload heap is used here for 
	//// code simplicity and because there are very few verts to actually transfer.
	//// 创建m_vertexBuffer，该对象既能够访问内存又能够访问显存
	//// 使用该对象读取内存中的数据并传输到显存

	//ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//	&defaultHeap,
	//	D3D12_HEAP_FLAG_NONE,
	//	&defaultBufferDesc,
	//	D3D12_RESOURCE_STATE_COPY_DEST,
	//	nullptr,
	//	IID_PPV_ARGS(&vertexBuffer)));

	//// 同时创建一个资源和一个隐式堆，使得堆足够大以包含整个资源，同时资源被映射到堆
	//// 此处创建的堆类型为UPLOAD堆，该堆用于将内存中的数据传输到显存
	//ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//	&uploadHeap,
	//	D3D12_HEAP_FLAG_NONE,
	//	&defaultBufferDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&uploadBuffer)));

	////获取 IndexBuffer footprint
	//D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint;
	//UINT64 totalBytes = 0;
	//d3dDevice->GetCopyableFootprints(&defaultBufferDesc, 0, 1, 0, &bufferFootprint, nullptr, nullptr, &totalBytes);

	////映射内存地址,并把数据拷贝到IndexBufferUploader里
	//void* ptr_index = nullptr;
	//uploadBuffer->Map(0, nullptr, &ptr_index);
	//memcpy(reinterpret_cast<char*>(ptr_index) + bufferFootprint.Offset, data, size);
	//uploadBuffer->Unmap(0, nullptr);

	////拷贝，把UploaderHeap里的数据拷贝到defaultHeap中
	//commandList->CopyBufferRegion(vertexBuffer.Get(), 0, uploadBuffer.Get(), 0, totalBytes);

	////为IndexBufferGPU插入资源屏障
	//D3D12_RESOURCE_BARRIER barrier;
	//memset(&barrier, 0, sizeof(barrier));
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier.Transition.pResource = vertexBuffer.Get();
	//barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	//barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	//commandList->ResourceBarrier(1, &barrier);

	//方法三
	//D3D12_HEAP_PROPERTIES  uploadHeap;
	//memset(&uploadHeap, 0, sizeof(uploadHeap));
	//auto descSize = CD3DX12_RESOURCE_DESC::Buffer(size);
	//uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
	//// 同时创建一个资源和一个隐式堆，使得堆足够大以包含整个资源，同时资源被映射到堆
	//// 此处创建的堆类型为UPLOAD堆，该堆用于将内存中的数据传输到显存
	//ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//&uploadHeap,
	//D3D12_HEAP_FLAG_NONE,
	//&descSize,
	//D3D12_RESOURCE_STATE_GENERIC_READ,
	//nullptr,
	//IID_PPV_ARGS(&vertexBuffer)));

	//// 将内存中的资源拷贝到以pVertexDataBegin为开始地址的显存中
	//// Copy the triangle data to the vertex buffer.
	//UINT8* pVertexDataBegin;
	//CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	//ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	//memcpy(pVertexDataBegin, data, size);
	//vertexBuffer->Unmap(0, nullptr);
}

void DxApp::CreateVertexAndIndexBuffer()
{
	for (auto i = 0; i < TEngine::staticMeshDatas.size(); i++)
	{
		staticMeshIndicesNums.push_back(TEngine::staticMeshDatas[i].indiceNum);

		auto subVertices = TEngine::staticMeshDatas[i].vertices;
		for(auto j=0;j< subVertices.size();j++)
		{ 
			allVertices.push_back(Vertex{ subVertices[j].position,subVertices[j].normal,subVertices[j].uv,subVertices[j].color });
		}
		auto subIndices = TEngine::staticMeshDatas[i].indices;
		for (auto j = 0; j < subIndices.size(); j++)
		{
			allIndices.push_back(subIndices[j]);
		}
	}

	const UINT verticesSize = (UINT)allVertices.size() * sizeof(Vertex);
	CreateDefaultHeapBuffer(commandList.Get(), allVertices.data(), verticesSize, vertexBuffer);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = verticesSize;

	const UINT indicesSize = (UINT)allIndices.size() * sizeof(std::uint16_t);
	CreateDefaultHeapBuffer(commandList.Get(), allIndices.data(), indicesSize, indexBuffer);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = indicesSize;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;


	//commandList记录了命令但是在提交到commandQueue之前就被reset了，所以需要强制提交一次并等待执行完毕
	ThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

//先建堆再建描述符
void DxApp::CreateCbvSrvUavDescriptor()
{
	// Describe and create a constant buffer view (CBV) descriptor heap.
	// Flags indicate that this descriptor heap can be bound to the pipeline 
	// and that descriptors contained in it can be referenced by a root table.
	// NumDescripters为1的含义为该常量缓冲视图描述符堆能够绑定到渲染管线上
	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc;
	// 描述符堆中的CBV、SRV、UAV总数，2个SRV与多个CBV（包含一个材质CBV）
	cbvSrvUavHeapDesc.NumDescriptors = TEngine::staticMeshDatas.size() + 4 + 1;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvUavHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&cbvSrvUavHeap)));

	objectConstantBuffer = std::make_unique<UploadHeapConstantBuffer<ObjectConstant>>(d3dDevice.Get(), TEngine::staticMeshDatas.size());
	materialConstantBuffer = std::make_unique<UploadHeapConstantBuffer<MaterialConstant>>(d3dDevice.Get(), 1);
	cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cbvSrvUavHeapHandle = cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	objectConstantBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, 0);
	for (auto i = 1; i < TEngine::staticMeshDatas.size(); i++)
	{
		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		objectConstantBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, i);
	}

	cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
	//第三个参数表示在一个uploadBuffer中的偏移大小，此处无偏移
	materialConstantBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, 0);
	MaterialConstant matConstant;
	matConstant.diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	matConstant.fresnelR0 = { 0.01f, 0.01f, 0.01f };
	matConstant.roughness = 0.25f;
	materialConstantBuffer->CopyData(0, matConstant);

	auto diffuseTex1 = textures["diffuseTex1"]->Resource;
	auto normalTex1 = textures["normalTex1"]->Resource;
	auto diffuseTex2 = textures["diffuseTex2"]->Resource;
	auto normalTex2 = textures["normalTex2"]->Resource;

	cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = diffuseTex1->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	d3dDevice->CreateShaderResourceView(diffuseTex1.Get(), &srvDesc, cbvSrvUavHeapHandle);

	cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
	srvDesc.Format = normalTex1->GetDesc().Format;
	d3dDevice->CreateShaderResourceView(normalTex1.Get(), &srvDesc, cbvSrvUavHeapHandle);

	cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
	srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc1.Format = diffuseTex2->GetDesc().Format;
	srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc1.Texture2D.MostDetailedMip = 0;
	srvDesc1.Texture2D.MipLevels = -1;
	d3dDevice->CreateShaderResourceView(diffuseTex2.Get(), &srvDesc1, cbvSrvUavHeapHandle);

	cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
	srvDesc.Format = normalTex2->GetDesc().Format;
	d3dDevice->CreateShaderResourceView(normalTex2.Get(), &srvDesc, cbvSrvUavHeapHandle);
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
	// 将fence的值更新为1
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceParameter));
	fenceValue++;

	// Wait until the previous frame is finished.
	if (fence->GetCompletedValue() < fenceParameter)
	{
		//如果围栏到达指定点则触发（即等队列执行完毕）
		//指定围栏到达某个值的时候应当触发的事件
		ThrowIfFailed(fence->SetEventOnCompletion(fenceParameter, fenceEvent));
		//如果等待的对象有信号则立即返回
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

//初始化dx12
bool DxApp::InitDirectx12()
{
	//枚举适配器
	EnumAdapter();
	CreateCommandObjects();
	//创建交换链
	CreateSwapChain();
	//创建渲染目标视图和深度模板视图的描述符堆
	CreateRtvAndDsvDescriptorHeap();
	//创建深度模板视图
	CreateDepthStencil();
	//加载纹理
	LoadTexture();
	//创建根签名
	CreateRootSignature();

	CreateCbvSrvUavDescriptor();
	//创建顶点和索引缓冲
	CreateVertexAndIndexBuffer();

	//创建渲染管线
	CreatePipelineState();
	//创建同步对象
	CreateSynObject();

	return true;
}

void DxApp::RenderTick()
{
	OnUpdate();
	OnRender();
}




