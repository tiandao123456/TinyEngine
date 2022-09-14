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

//����worldViewProj����
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

	// ���ø�ǩ�������������Լ���������
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	//���ӿڵ���Ⱦ���ߵĹ�դ���׶�
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	// ������̨���彫��ʹ����Ϊ��ȾĿ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto x = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// ֪ͨ������������Ҫͬ������Դ�Ķ�η��� 
	// ����Դ���������ͬ������Դ���ϼ�������Դ��״̬ת��
	// GPU��ĳ��ALU�Ը���Դ������ֻ��������ĳ��ALU������ֻд������ʹ����Դ���ϾͿ��Ը���Դһ��״̬
	// ����Դ���ֻ��״̬ʱ������ĳЩALU���Խ��ж���Դ���в����ˣ�ֻдͬ��
	commandList->ResourceBarrier(1, &x);

	ID3D12DescriptorHeap* ppHeaps[] = { cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(cbvHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetGraphicsRootDescriptorTable(0, handle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	for (auto i = 0; i < TEngine::staticMeshDatas.size(); i++)
	{
		handle.Offset(cbvSrvUavDescriptorSize * i);
		// ��һ��������ʾ�󶨵Ĳ�λ�ţ������ڼĴ���b0�ϣ�����������const buffer������Ҫƫ��
		commandList->SetGraphicsRootDescriptorTable(0, handle);
		if (i==0)
			// ����һ��ʵ������һ������Ϊ��������
			commandList->DrawIndexedInstanced((UINT)staticMeshIndicesNums[0], 1, 0, 0, 0);
		else
			commandList->DrawIndexedInstanced((UINT)staticMeshIndicesNums[i], 1, (UINT)staticMeshIndicesNums[i-1], TEngine::staticMeshDatas[i-1].vertices.size(), 0);
	}
	auto y = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	// ������̨�������ڽ�������
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
		//��ȡ���Խӿ�
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		//�������Թ���
		debugController->EnableDebugLayer();
	}
#endif

	//����DXGI������ʹ��DXGI����������DXGI����
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	//����һ����ʾ��ʾ���������豸
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&d3dDevice));

	//�������Ӳ����ʾ������ʧ�ܣ���ö�������ʾ������
	//���������һ�����Խ�����Ⱦ���ߵ�һϵ�в���
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

//����������У������б��Լ����������
//cpu�ܹ��������б��е������ύ�����������
//���cpu�ܹ�������������б����Ӷ���ĵ�����
//��gpu������������ȡ����ִ��

//���������
//1�����������б��е������ύ���������֮��
//2����������б��е������ʹ֮����ԭ������������Ƶ�����ڴ�������������
void DxApp::CreateCommandObjects()
{
}

void DxApp::CreateSwapChain()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//���Կ�����������������У����������GPUά��
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

	// ����������������
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
		commandQueue.Get(),        // ��������Ҫ������в���ˢ�£��������������һ��ָ��ָʾ����������ǰ��̨����Ľ���
		TEngine::mhMainWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	//�ô��ڽ�������Ӧalt-enter���У�������ϣ�
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(TEngine::mhMainWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&dxgiSwapChain));
	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

void DxApp::CreateRtvAndCbvAndDsv()
{
	// descriptor��ʾ��Դ���Դ��еĴ�ŵ�ַ������Դ�����Ƕ�������ȣ�
	// heaps���Ǵ��descriptor��һƬ�ڴ�
	{
		// ������ȾĿ����ͼ����ȾĿ����ͼ����Ⱦ���ߵ�����Ŀ�������λ�ã���Ϊ��Ⱦ�������ղ����������Ļ�ϣ������������ȾĿ����ͼ�ϣ�
		// �����Ļ��ȡ��ȾĿ����ͼ�����ݲ�����
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = swapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

		//����RTV�������Ĵ�С
		rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		// NumDescriptersΪ1�ĺ���Ϊ�ó���������ͼ���������ܹ��󶨵���Ⱦ������
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = TEngine::staticMeshDatas.size();
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));
	}

	// Create frame resources.
	{
		//�����ȾĿ����ͼ�������ѵ���ʼλ��
		//		| RTV Descriptor Heap |
		//		|---------------------|
		//0		| BackBuffer0's rtv   |
		//		|---------------------|
		//1		| BackBuffer1's rtv   |
		//		|---------------------|
		//���洴������ȾĿ����ͼ���������ѣ�֮���ٸöѵĻ������ٴ�����ȾĿ����ͼ
		//������Ҫ���õ��ѵ���ʼ��ַ��֮������������Ĵ�С����ƫ�ƴ�����ȾĿ����ͼ
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < swapChainBufferCount; n++)
		{
			ThrowIfFailed(dxgiSwapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
			d3dDevice->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, rtvDescriptorSize);
		}
	}

	//�������ģ���
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));

	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void DxApp::CreateRootSignature()
{
	//������ǩ�����ø�ǩ������һ������������������������һ��CBV
	//�����������������ѵĹ�ϵ����������ʵ�������������ѵ��ӷ�Χ
	//����SRV�����Ҫ����shader�У�������뵽���������У�descriptor table��
	//Ȼ��CBVͬ��Ҳ���Էŵ����������У�Ҳ���Բ���ֱ�Ӵ��䵽shader��
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	//������ǩ����׼������
	//�����������ķ�Χ
	CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];

	//��һ����������������������
	//�ڶ�����������������������
	//���������������Ĵ���ӳ��,������ΪCBV 0����������ӳ�䵽�Ĵ���b0�У���shader��Ҳ���Կ�����Ӧ��ϵ
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//��������ĳ�ʼ����������������ֻ��һ������
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	// ����Ⱦ����ĳЩ�׶εľܽӷ��ʺ��������
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	//��ǩ������
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	//��ǩ�������л�
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	//������ǩ��
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

	//ʵʱ������ɫ��
	ThrowIfFailed(D3DCompileFromFile(L"Shaders\\color.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"Shaders\\color.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	//���嶥������벼��
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		//����λ�ô�0ƫ�ƴ���ʼ��������ɫ�ӵ�12���ֽڣ�x,y,z����ʼ
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//�������״̬
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);//���ģ��״̬
	psoDesc.SampleMask = UINT_MAX;
	//���ͼ��ϻ������ɫ������ͼԪ���˴�����ͼԪΪ������
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//������Դ�Ķ��ز������˴�����Ϊ1�������ж��ز���
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	// Create the command list.
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	// �����б�����������¼����ģ���������û������ɼ�¼����˹ر�
	// ThrowIfFailed(commandList->Close());
	//���ں������������Բ���close
}
void DxApp::CreateDefaultHeapBuffer(ID3D12GraphicsCommandList* cmdList, const void* data, const int size, ComPtr<ID3D12Resource> &vertexBuffer)
{
	//����Դ���ϴ��ѿ�����Ĭ�϶�
	//�������ַ��������У�������d3dx12�ļ��汾����
	//��ͬ�汾�к�����ʵ�ֲ�ͬ
	//����һ
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
	auto barrierFromCommonToDest = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &barrierFromCommonToDest);
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

	auto barrierFromDestToRead = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &barrierFromDestToRead);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.
	vertexBuffer = defaultBuffer;

	////������
	//D3D12_HEAP_PROPERTIES defaultHeap;
	//memset(&defaultHeap, 0, sizeof(defaultHeap));
	//defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

	//D3D12_HEAP_PROPERTIES  uploadHeap;
	//memset(&uploadHeap, 0, sizeof(uploadHeap));
	//uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

	////����VertexBuffer����Դ����
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
	//// ����m_vertexBuffer���ö�����ܹ������ڴ����ܹ������Դ�
	//// ʹ�øö����ȡ�ڴ��е����ݲ����䵽�Դ�

	//ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//	&defaultHeap,
	//	D3D12_HEAP_FLAG_NONE,
	//	&defaultBufferDesc,
	//	D3D12_RESOURCE_STATE_COPY_DEST,
	//	nullptr,
	//	IID_PPV_ARGS(&vertexBuffer)));

	//// ͬʱ����һ����Դ��һ����ʽ�ѣ�ʹ�ö��㹻���԰���������Դ��ͬʱ��Դ��ӳ�䵽��
	//// �˴������Ķ�����ΪUPLOAD�ѣ��ö����ڽ��ڴ��е����ݴ��䵽�Դ�
	//ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//	&uploadHeap,
	//	D3D12_HEAP_FLAG_NONE,
	//	&defaultBufferDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&uploadBuffer)));

	////��ȡ IndexBuffer footprint
	//D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint;
	//UINT64 totalBytes = 0;
	//d3dDevice->GetCopyableFootprints(&defaultBufferDesc, 0, 1, 0, &bufferFootprint, nullptr, nullptr, &totalBytes);

	////ӳ���ڴ��ַ,�������ݿ�����IndexBufferUploader��
	//void* ptr_index = nullptr;
	//uploadBuffer->Map(0, nullptr, &ptr_index);
	//memcpy(reinterpret_cast<char*>(ptr_index) + bufferFootprint.Offset, data, size);
	//uploadBuffer->Unmap(0, nullptr);

	////��������UploaderHeap������ݿ�����defaultHeap��
	//commandList->CopyBufferRegion(vertexBuffer.Get(), 0, uploadBuffer.Get(), 0, totalBytes);

	////ΪIndexBufferGPU������Դ����
	//D3D12_RESOURCE_BARRIER barrier;
	//memset(&barrier, 0, sizeof(barrier));
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier.Transition.pResource = vertexBuffer.Get();
	//barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	//barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	//commandList->ResourceBarrier(1, &barrier);

	//������
	//D3D12_HEAP_PROPERTIES  uploadHeap;
	//memset(&uploadHeap, 0, sizeof(uploadHeap));
	//auto descSize = CD3DX12_RESOURCE_DESC::Buffer(size);
	//uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
	//// ͬʱ����һ����Դ��һ����ʽ�ѣ�ʹ�ö��㹻���԰���������Դ��ͬʱ��Դ��ӳ�䵽��
	//// �˴������Ķ�����ΪUPLOAD�ѣ��ö����ڽ��ڴ��е����ݴ��䵽�Դ�
	//ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//&uploadHeap,
	//D3D12_HEAP_FLAG_NONE,
	//&descSize,
	//D3D12_RESOURCE_STATE_GENERIC_READ,
	//nullptr,
	//IID_PPV_ARGS(&vertexBuffer)));

	//// ���ڴ��е���Դ��������pVertexDataBeginΪ��ʼ��ַ���Դ���
	//// Copy the triangle data to the vertex buffer.
	//UINT8* pVertexDataBegin;
	//CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	//ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	//memcpy(pVertexDataBegin, data, size);
	//vertexBuffer->Unmap(0, nullptr);
}

void DxApp::CreateVertexBuffer()
{
	for (auto i = 0; i < TEngine::staticMeshDatas.size(); i++)
	{
		staticMeshIndicesNums.push_back(TEngine::staticMeshDatas[i].indiceNum);

		auto subVertices = TEngine::staticMeshDatas[i].vertices;
		for(auto j=0;j< subVertices.size();j++)
		{ 
			allVertices.push_back(Vertex{ subVertices[j].position,subVertices[j].color });
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


	//commandList��¼����������ύ��commandQueue֮ǰ�ͱ�reset�ˣ�������Ҫǿ���ύһ�β��ȴ�ִ�����
	ThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void DxApp::CreateConstantBuffer()
{
	objectConstantBuffer = std::make_unique<UploadHeapConstantBuffer<ObjectConstant>>(d3dDevice.Get(), TEngine::staticMeshDatas.size());
	cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cbvHeapHandle = cbvHeap->GetCPUDescriptorHandleForHeapStart();
	objectConstantBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvHeapHandle, 0);
	for (auto i = 1; i < TEngine::staticMeshDatas.size(); i++)
	{
		cbvHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		objectConstantBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvHeapHandle, i);
	}
}
//���������ģ�建��Ļ�����������ᵲס�Ȼ��Ƶ����壬�����Ǹ���ʵ�ʵ���Ȼ���
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

	CD3DX12_HEAP_PROPERTIES heapProperties2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC tex2D = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mClientWidth, mClientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties2,
		D3D12_HEAP_FLAG_NONE,
		&tex2D,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)));

	d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
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
	// ��fence��ֵ����Ϊ1
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceParameter));
	fenceValue++;

	// Wait until the previous frame is finished.
	if (fence->GetCompletedValue() < fenceParameter)
	{
		//���Χ������ָ�����򴥷������ȶ���ִ����ϣ�
		//ָ��Χ������ĳ��ֵ��ʱ��Ӧ���������¼�
		ThrowIfFailed(fence->SetEventOnCompletion(fenceParameter, fenceEvent));
		//����ȴ��Ķ������ź�����������
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

//��ʼ��dx12
bool DxApp::InitDirectx12()
{
	//ö��������
	EnumAdapter();
	//����������
	CreateSwapChain();
	//������ȾĿ����ͼ�ͳ���������ͼ
	CreateRtvAndCbvAndDsv();
	//������ǩ��
	CreateRootSignature();
	//������Ⱦ����
	CreatePipelineState();
	//�����������������
	CreateVertexBuffer();
	//������������
	CreateConstantBuffer();
	CreateDepthStencil();
	//����ͬ������
	CreateSynObject();

	return true;
}

void DxApp::RenderTick()
{
	OnUpdate();
	OnRender();
}




