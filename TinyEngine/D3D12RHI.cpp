#include "stdafx.h"
#include "D3D12RHI.h"

void D3D12RHI::RHIDrawSceneToShadow()
{
	// ���ø�ǩ�������������Լ���������
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvUavHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandle(cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandleCopy = cbvSrvUavHeapHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandleCopyAgain = cbvSrvUavHeapHandle;
	for (auto i = 0; i < cbvNums + srvNums; i++)
		cbvSrvUavHeapHandle.Offset(cbvSrvUavDescriptorSize);
	//���ø�table��5�Ų�������0��ʼ��
	commandList->SetGraphicsRootDescriptorTable(4, cbvSrvUavHeapHandle);
	commandList->RSSetViewports(1, &shadowMapViewport);
	commandList->RSSetScissorRects(1, &shadowMapScissorRect);

	auto resourceFromReadToWrite = CD3DX12_RESOURCE_BARRIER::Transition(shadowMapResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandList->ResourceBarrier(1, &resourceFromReadToWrite);

	// Clear the back buffer and depth buffer.
	commandList->ClearDepthStencilView(shadowDsvHeapHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	commandList->OMSetRenderTargets(0, nullptr, false, &shadowDsvHeapHandle);

	commandList->SetPipelineState(shadowPassPipelineState.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	//����2�Ų���������洢�еƹ��vp��vpt�����Լ������vp
	for (auto i = 0; i < cbvNums; i++)
		cbvSrvUavHeapHandleCopy.Offset(cbvSrvUavDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(1, cbvSrvUavHeapHandleCopy);

	for (auto i = 0; i < cbvNums; i++)
	{
		//�Ѿ��ƫ�ƶ��ǵ��ӵ�
		//�����д��
		//cbvSrvUavHeapHandleCopyAgain.Offset(cbvSrvUavDescriptorSize * i);
		if (i == 0){}
		else
		{
			cbvSrvUavHeapHandleCopyAgain.Offset(cbvSrvUavDescriptorSize);
		}
		//��һ��������ʾ��ǩ���е������±꣬0��ʾΪ��ǩ���ĵ�һ��Ԫ��
		//��ǩ���еĵ�һ��Ԫ�ض�Ӧ�Ŷ��CBV������Ҫƫ��
		commandList->SetGraphicsRootDescriptorTable(0, cbvSrvUavHeapHandleCopyAgain);
		// ����һ��ʵ������һ������Ϊ�����������������ĸ������ֱ�Ϊ�����Ͷ����������еĶ�Ӧ�±�λ��
		commandList->DrawIndexedInstanced((UINT)staticMeshIndicesNums[i], 1, recordIndexStartPosition[i], recondVertexStartPosition[i], 0);
	}
	auto resourceFromWriteToRead = CD3DX12_RESOURCE_BARRIER::Transition(shadowMapResource.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &resourceFromWriteToRead);
}

void D3D12RHI::RHIPopulateCommandList()
{
	ThrowIfFailed(commandAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), basePassPipelineState.Get()));

	RHIDrawSceneToShadow();
	//���ӿڵ���Ⱦ���ߵĹ�դ���׶�
	commandList->RSSetViewports(1, &baseViewport);
	commandList->RSSetScissorRects(1, &baseScissorRect);

	// ������̨���彫��ʹ����Ϊ��ȾĿ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto resourceFromPresentToTarget = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// ֪ͨ������������Ҫͬ������Դ�Ķ�η��� 
	// ����Դ���������ͬ������Դ���ϼ�������Դ��״̬ת��
	// GPU��ĳ��ALU�Ը���Դ������ֻ��������ĳ��ALU������ֻд������ʹ����Դ���ϾͿ��Ը���Դһ��״̬
	// ����Դ���ֻ��״̬ʱ������ĳЩALU���Խ��ж���Դ���в����ˣ�ֻдͬ��
	commandList->ResourceBarrier(1, &resourceFromPresentToTarget);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->SetPipelineState(basePassPipelineState.Get());

	// ���ø�ǩ�������������Լ���������
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvUavHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandle(cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandleCopy = cbvSrvUavHeapHandle;

	//���������vp����
	for (auto i = 0; i < cbvNums; i++)
		cbvSrvUavHeapHandleCopy.Offset(1, cbvSrvUavDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(1, cbvSrvUavHeapHandleCopy);
	cbvSrvUavHeapHandleCopy.Offset(1, cbvSrvUavDescriptorSize);

	for (auto i = 0; i < cbvNums; i++)
	{
		if (i == 0) {}
		else
		{
			cbvSrvUavHeapHandle.Offset(cbvSrvUavDescriptorSize);
			cbvSrvUavHeapHandleCopy.Offset(1, cbvSrvUavDescriptorSize);
		}
		//��һ��������ʾ��ǩ���е������±꣬0��ʾΪ��ǩ���ĵ�һ��Ԫ��
		//��ǩ���еĵ�һ��Ԫ�ض�Ӧ�Ŷ��CBV������Ҫƫ��
		commandList->SetGraphicsRootDescriptorTable(0, cbvSrvUavHeapHandle);
		commandList->SetGraphicsRootDescriptorTable(2, cbvSrvUavHeapHandleCopy);
		cbvSrvUavHeapHandleCopy.Offset(1, cbvSrvUavDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(3, cbvSrvUavHeapHandleCopy);
		commandList->DrawIndexedInstanced((UINT)staticMeshIndicesNums[i], 1, recordIndexStartPosition[i], recondVertexStartPosition[i], 0);
	}
	auto resourceFromTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	// ������̨�������ڽ�������
	commandList->ResourceBarrier(1, &resourceFromTargetToPresent);
	ThrowIfFailed(commandList->Close());
}

void D3D12RHI::CalculateCameraViewProj()
{
	auto camera = SceneManage::GetInstance().GetCameraActor();
	auto theNumOneCamera = camera[camera.begin()->first];

	XMVECTOR pos = XMVectorSet(theNumOneCamera.location[0], theNumOneCamera.location[1], theNumOneCamera.location[2], 1.0f);
	XMVECTOR target = XMVectorSet(theNumOneCamera.target[0], theNumOneCamera.target[1], theNumOneCamera.target[2], 1.0f);
	XMVECTOR up = XMVectorSet(0.0, 0.0f, 1.0f, 0.0f);
	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.5f * Pi, theNumOneCamera.aspect, 1.0f, 1000.0f);
	cameraViewProjMatrix = view * proj;
}

void D3D12RHI::CalculateLightViewProjTex()
{
	auto light = SceneManage::GetInstance().GetLight();

	XMFLOAT3 center = XMFLOAT3(0.0, 0.0, 0.0);
	XMFLOAT3 pos = XMFLOAT3(light.location[0], light.location[1], light.location[2]);
	XMVECTOR lightPos = XMLoadFloat3(&pos);
	XMVECTOR targetPos = XMLoadFloat3(&center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - 800;
	float b = sphereCenterLS.y - 800;
	float n = sphereCenterLS.z - 800;
	float r = sphereCenterLS.x + 800;
	float t = sphereCenterLS.y + 800;
	float f = sphereCenterLS.z + 800;

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	lightViewProjMatrix = lightView * lightProj;
	lightViewProjTexMatrix = lightView * lightProj * T;
}

void D3D12RHI::RHIUpdate()
{
	int position = 0;
	const auto& staticMeshActor = SceneManage::GetInstance().GetStaticMeshActor();
	for (auto iter = staticMeshActor.begin(); iter != staticMeshActor.end(); iter++)
	{
		for (auto i = 0; i < iter->second.worldMatrix.size(); i++)
		{
			XMMATRIX world =
			{
				iter->second.worldMatrix[i][0],iter->second.worldMatrix[i][1],iter->second.worldMatrix[i][2],iter->second.worldMatrix[i][3],
				iter->second.worldMatrix[i][4],iter->second.worldMatrix[i][5],iter->second.worldMatrix[i][6],iter->second.worldMatrix[i][7],
				iter->second.worldMatrix[i][8],iter->second.worldMatrix[i][9],iter->second.worldMatrix[i][10],iter->second.worldMatrix[i][11],
				iter->second.worldMatrix[i][12],iter->second.worldMatrix[i][13],iter->second.worldMatrix[i][14],iter->second.worldMatrix[i][15]
			};
			WorldMatrix worldMatrixStruct;
			XMStoreFloat4x4(&worldMatrixStruct.worldMatrix, XMMatrixTranspose(world));
			worldMatrixBuffer->CopyData(position++, worldMatrixStruct);
		}
	}

	//ʵʱ����
	CalculateCameraViewProj();
	CalculateLightViewProjTex();
	ConstMatrix constMatrix;
	XMStoreFloat4x4(&constMatrix.cameraVPMatrix, XMMatrixTranspose(cameraViewProjMatrix));
	XMStoreFloat4x4(&constMatrix.lightVPMatrix, XMMatrixTranspose(lightViewProjMatrix));
	XMStoreFloat4x4(&constMatrix.lightVPTexMatrix, XMMatrixTranspose(lightViewProjTexMatrix));

	constMatrixBuffer->CopyData(0, constMatrix);
}

void D3D12RHI::RHIRender()
{
	// Record all the commands we need to render the scene into the command list.
	RHIPopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	// Present the frame.
	ThrowIfFailed(dxgiSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3D12RHI::RHIStartDebug()
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
}

void D3D12RHI::RHIEnumAdapter()
{
	//����DXGI������ʹ��DXGI����������DXGI����
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	//����һ����ʾ��ʾ���������豸
	HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));

	//�������Ӳ����ʾ������ʧ�ܣ���ö�������ʾ������
	//���������һ�����Խ�����Ⱦ���ߵ�һϵ�в���
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)));
	}
}

//����������У������б��Լ����������
//cpu�ܹ��������б��е������ύ�����������
//���cpu�ܹ�������������б����Ӷ���ĵ�����
//��gpu������������ȡ����ִ��

//���������
//1�����������б��е������ύ���������֮��
//2����������б��е������ʹ֮����ԭ������������Ƶ�����ڴ�������������
void D3D12RHI::RHICreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//���Կ�����������������У����������GPUά��
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	// Create the command list.
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), basePassPipelineState.Get(), IID_PPV_ARGS(&commandList)));
}

void D3D12RHI::RHICreateSwapChain(UINT bufferCount, Dataformat dFormat)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Width = mClientWidth;
	swapChainDesc.Height = mClientHeight;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	switch(dFormat)
	{
	case R8G8B8A8:
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	default:
		break;
	}

	HWND mhMainWnd = Win::GetInstance().mhMainWnd;
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),// ��������Ҫ������в���ˢ�£��������������һ��ָ��ָʾ����������ǰ��̨����Ľ���
		mhMainWnd, &swapChainDesc, nullptr, nullptr, &swapChain));

	//�ô��ڽ�������Ӧalt-enter���У�������ϣ�
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&dxgiSwapChain));
	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

//����rtv
void D3D12RHI::RHICreateRenderTarget(UINT renderTargetNums)
{
	// descriptor��ʾ��Դ���Դ��еĴ�ŵ�ַ������Դ�����Ƕ�������ȣ�
	// heaps���Ǵ��descriptor��һƬ�ڴ�
	// ������ȾĿ����ͼ����ȾĿ����ͼ����Ⱦ���ߵ�����Ŀ�������λ�ã���Ϊ��Ⱦ�������ղ����������Ļ�ϣ������������ȾĿ����ͼ�ϣ�
	// �����Ļ��ȡ��ȾĿ����ͼ�����ݲ�����
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = renderTargetNums;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));
	// ����RTV�������Ĵ�С
	rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�����ȾĿ����ͼ�������ѵ���ʼλ��
	//		| RTV Descriptor Heap |
	//		|---------------------|
	//0		| BackBuffer0's rtv   |
	//		|---------------------|
	//1		| BackBuffer1's rtv   |
	//		|---------------------|
	//���洴������ȾĿ����ͼ���������ѣ�֮���ڸöѵĻ������ٴ�����ȾĿ����ͼ
	//������Ҫ���õ��ѵ���ʼ��ַ��֮������������Ĵ�С����ƫ�ƴ�����ȾĿ����ͼ
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	// Create a RTV for each frame.
	for (UINT n = 0; n < renderTargetNums; n++)
	{
		ThrowIfFailed(dxgiSwapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
		d3dDevice->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize);
	}
}

void D3D12RHI::RHICreateDepthStencil(Dataformat dFormat, UINT shadowMapWidth, UINT shadowMapHeight)
{
	//�������ģ���
	//���/ģ��Ѵ�СΪ2����Ϊ��һ��shadowMap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));
	//���������ģ�建��Ļ�����������ᵲס�Ȼ��Ƶ����壬�����Ǹ���ʵ�ʵ���Ȼ���

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

	//shadowMap�����Դ
	shadowMapViewport = { 0.0f, 0.0f, (float)shadowMapWidth, (float)shadowMapHeight, 0.0f, 1.0f };
	shadowMapScissorRect = { 0, 0, (int)shadowMapWidth, (int)shadowMapHeight };

	D3D12_RESOURCE_DESC shadowMapResourceDesc;
	ZeroMemory(&shadowMapResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	shadowMapResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	shadowMapResourceDesc.Alignment = 0;
	shadowMapResourceDesc.Width = shadowMapWidth;
	shadowMapResourceDesc.Height = shadowMapHeight;
	shadowMapResourceDesc.DepthOrArraySize = 1;
	shadowMapResourceDesc.MipLevels = 1;
	shadowMapResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapResourceDesc.SampleDesc.Count = 1;
	shadowMapResourceDesc.SampleDesc.Quality = 0;
	shadowMapResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	shadowMapResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&shadowMapResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&shadowMapResource)));

	// ��shadow map��Ⱦ��dsv��
	D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapDesc;
	shadowMapDesc.Flags = D3D12_DSV_FLAG_NONE;
	shadowMapDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	shadowMapDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowMapDesc.Texture2D.MipSlice = 0;

	dsvHeapHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	shadowDsvHeapHandle = dsvHeapHandle;
	dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	shadowDsvHeapHandle.Offset(1, dsvDescriptorSize);
	d3dDevice->CreateDepthStencilView(shadowMapResource.Get(), &shadowMapDesc, shadowDsvHeapHandle);
}

//�����������������ѵĹ�ϵ����������ʵ�������������ѵ��ӷ�Χ
//srv�����Ҫ����shader�У�������뵽���������У�descriptor table��
//Ȼ��cbvͬ��Ҳ���Էŵ����������У�Ҳ���Բ���ֱ�Ӵ��䵽shader��
void D3D12RHI::RHICreateRootDescriptorTable()
{
	//���ݳ����е������������Ҫ��cbv��srv����
	const auto &staticMeshActor = SceneManage::GetInstance().GetStaticMeshActor();
	for (auto iter = staticMeshActor.begin(); iter != staticMeshActor.end(); iter++)
	{
		//cbv������
		cbvNums += iter->second.worldMatrix.size();
		srvNums += iter->second.worldMatrix.size() * 2;
	}
	//����shadow map
	srvNums += 1;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

	CD3DX12_DESCRIPTOR_RANGE1 ranges[5];
	//world����ȫ������b0�Ĵ�����
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//����һЩ�ı�û����ôƵ���ľ�����ƹ��vp����shadowTransform���������vp����
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	//��������ͼ��Դ��t0��
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	//�󶨷�����ͼ��Դ��t1��
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	//��shadow map��t2����
	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

	CD3DX12_ROOT_PARAMETER1 rootParameters[5];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);

	//����������
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

void D3D12RHI::LoadTexture()
{
	//���ļ����ж�ȡ����dds�ļ�
	auto diffuseTexBrick = std::make_unique<Texture>();
	diffuseTexBrick->name = "brickDiffuse";
	diffuseTexBrick->filename = L"./Resources/bricks.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), diffuseTexBrick->filename.c_str(),
		diffuseTexBrick->resource, diffuseTexBrick->uploadHeap));

	auto normalTexBrick = std::make_unique<Texture>();
	normalTexBrick->name = "brickNormal";
	normalTexBrick->filename = L"./Resources/bricks_nmap.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), normalTexBrick->filename.c_str(),
		normalTexBrick->resource, normalTexBrick->uploadHeap));

	auto diffuseTexTile = std::make_unique<Texture>();
	diffuseTexTile->name = "tileDiffuse";
	diffuseTexTile->filename = L"./Resources/tile.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), diffuseTexTile->filename.c_str(),
		diffuseTexTile->resource, diffuseTexTile->uploadHeap));

	auto normalTexTile = std::make_unique<Texture>();
	normalTexTile->name = "tileNormal";
	normalTexTile->filename = L"./Resources/tile_nmap.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(d3dDevice.Get(),
		commandList.Get(), normalTexTile->filename.c_str(),
		normalTexTile->resource, normalTexTile->uploadHeap));

	//��ֵ�Ա��棬name��Ӧuni_ptr
	textures[diffuseTexBrick->name] = std::move(diffuseTexBrick);
	textures[normalTexBrick->name] = std::move(normalTexBrick);
	textures[diffuseTexTile->name] = std::move(diffuseTexTile);
	textures[normalTexTile->name] = std::move(normalTexTile);
}

void D3D12RHI::RHICreateConstBufferAndShaderResource()
{
	// NumDescriptersΪ1�ĺ���Ϊ�ó���������ͼ���������ܹ��󶨵���Ⱦ������
	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc;
	// ���������е�cbv��srv��uav����
	cbvSrvUavHeapDesc.NumDescriptors = cbvNums + srvNums + 1;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvUavHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&cbvSrvUavHeap)));

	worldMatrixBuffer = std::make_unique<UploadHeapConstantBuffer<WorldMatrix>>(d3dDevice.Get(), cbvNums);
	constMatrixBuffer = std::make_unique<UploadHeapConstantBuffer<ConstMatrix>>(d3dDevice.Get(), 1);
	cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cbvSrvUavHeapHandle = cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();

	//�������cb����������Ӧģ�͵��������
	for (auto i = 0; i < cbvNums; i++)
	{
		worldMatrixBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, i);
		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
	}
	//����һ��cb����������ӦConstMatirx
	constMatrixBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, 0);

	LoadTexture();
	auto diffuseTexBrick = textures["brickDiffuse"]->resource;
	auto diffuseTexTile = textures["tileDiffuse"]->resource;
	auto normalTexBrick = textures["brickNormal"]->resource;
	auto normalTexTile = textures["tileNormal"]->resource;

	//�������sr��������
	for (auto i = 0; i < (srvNums - 1) / 4; i++)
	{
		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = diffuseTexBrick->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		d3dDevice->CreateShaderResourceView(diffuseTexBrick.Get(), &srvDesc, cbvSrvUavHeapHandle);

		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		srvDesc.Format = normalTexBrick->GetDesc().Format;
		d3dDevice->CreateShaderResourceView(normalTexBrick.Get(), &srvDesc, cbvSrvUavHeapHandle);

		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
		srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc1.Format = diffuseTexTile->GetDesc().Format;
		srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc1.Texture2D.MostDetailedMip = 0;
		srvDesc1.Texture2D.MipLevels = -1;
		d3dDevice->CreateShaderResourceView(diffuseTexTile.Get(), &srvDesc1, cbvSrvUavHeapHandle);

		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		srvDesc1.Format = normalTexTile->GetDesc().Format;
		d3dDevice->CreateShaderResourceView(normalTexTile.Get(), &srvDesc1, cbvSrvUavHeapHandle);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapHandle = cbvSrvUavHeapHandle;
	shadowMapHandle.Offset(1, cbvSrvUavDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	d3dDevice->CreateShaderResourceView(shadowMapResource.Get(), &srvDesc, shadowMapHandle);
}

void D3D12RHI::CreateDefaultHeapBuffer(ID3D12GraphicsCommandList* cmdList, const void* data, const int size, ComPtr<ID3D12Resource>& vertexBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

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

	// ת����Դ��״̬������Դ��upload buffer������default buffer
	auto barrierFromCommonToDest = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &barrierFromCommonToDest);
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

	//ת����Դ״̬����Դ��Ϊ�ɶ�
	auto barrierFromDestToRead = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &barrierFromDestToRead);
	vertexBuffer = defaultBuffer;
}

void D3D12RHI::RHISetVertexAndIndexBuffer()
{
	//���������������һ��������
	const auto& staticMeshActor = SceneManage::GetInstance().GetStaticMeshActor();
	
	int indexStartPosition = 0;
	int vertexStartPosition = 0;
	recondVertexStartPosition.push_back(vertexStartPosition);
	recordIndexStartPosition.push_back(indexStartPosition);

	for (auto iter = staticMeshActor.begin(); iter != staticMeshActor.end(); iter++)
	{
		for (auto i = 0; i < iter->second.worldMatrix.size(); i++)
		{
			staticMeshIndicesNums.push_back(iter->second.indiceNum);

			auto subVertices = iter->second.staticMesh.vertices;
			staticMeshVerticesNums.push_back(subVertices.size());
			for (auto j = 0; j < subVertices.size(); j++)
			{
				allVertices.push_back(Vertex{ subVertices[j].position,subVertices[j].normal,subVertices[j].uv });
			}

			auto subIndices = iter->second.staticMesh.indices;
			for (auto j = 0; j < subIndices.size(); j++)
			{
				allIndices.push_back(subIndices[j]);
			}

			//��¼ÿ������Ķ���������������е���ʼ�±�
			indexStartPosition += iter->second.indiceNum;
			vertexStartPosition += iter->second.staticMesh.vertices.size();
			recondVertexStartPosition.push_back(vertexStartPosition);
			recordIndexStartPosition.push_back(indexStartPosition);
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
void D3D12RHI::RHICreatePipeLineState()
{
	//���������������ܹ��������hlsl�ļ�
	shaders["MainShaderVS"] = d3dUtil::CompileShader(L"Shaders\\MainShader.hlsl", nullptr, "VS", "vs_5_1");
	shaders["MainShaderPS"] = d3dUtil::CompileShader(L"Shaders\\MainShader.hlsl", nullptr, "PS", "ps_5_1");
	//���嶥������벼��
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		//����λ�ô�0ƫ�ƴ���ʼ��������ɫ�ӵ�12���ֽڣ�x,y,z����ʼ
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC basePassPsoDesc = {};
	basePassPsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	basePassPsoDesc.pRootSignature = rootSignature.Get();
	basePassPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(shaders["MainShaderVS"]->GetBufferPointer()),
		shaders["MainShaderVS"]->GetBufferSize()
	};
	basePassPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(shaders["MainShaderPS"]->GetBufferPointer()),
		shaders["MainShaderPS"]->GetBufferSize()
	};
	basePassPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//�������״̬
	basePassPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	basePassPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);//���ģ��״̬
	basePassPsoDesc.SampleMask = UINT_MAX;
	//���ͼ��ϻ������ɫ������ͼԪ���˴�����ͼԪΪ������
	basePassPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	basePassPsoDesc.NumRenderTargets = 1;
	basePassPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//������Դ�Ķ��ز������˴�����Ϊ1�������ж��ز���
	basePassPsoDesc.SampleDesc.Count = 1;

	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&basePassPsoDesc, IID_PPV_ARGS(&basePassPipelineState)));

	// PSO for shadowmap.
	shaders["GenerateShadowVS"] = d3dUtil::CompileShader(L"Shaders\\GenerateShadow.hlsl", nullptr, "VS", "vs_5_1");
	shaders["GenerateShadowPS"] = d3dUtil::CompileShader(L"Shaders\\GenerateShadow.hlsl", nullptr, "PS", "ps_5_1");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPassPsoDesc;
	ZeroMemory(&shadowPassPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	shadowPassPsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	shadowPassPsoDesc.pRootSignature = rootSignature.Get();
	shadowPassPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(shaders["GenerateShadowVS"]->GetBufferPointer()),
		shaders["GenerateShadowVS"]->GetBufferSize()
	};
	shadowPassPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(shaders["GenerateShadowPS"]->GetBufferPointer()),
		shaders["GenerateShadowPS"]->GetBufferSize()
	};
	shadowPassPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	shadowPassPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	shadowPassPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	shadowPassPsoDesc.SampleMask = UINT_MAX;
	shadowPassPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	shadowPassPsoDesc.NumRenderTargets = 1;
	shadowPassPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	shadowPassPsoDesc.SampleDesc.Count = 1;
	shadowPassPsoDesc.SampleDesc.Quality = 0;
	shadowPassPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowPassPsoDesc.RasterizerState.DepthBias = 100000;
	shadowPassPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	shadowPassPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	shadowPassPsoDesc.pRootSignature = rootSignature.Get();
	// Shadow map pass does not have a render target.
	shadowPassPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	shadowPassPsoDesc.NumRenderTargets = 0;

	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowPassPsoDesc, IID_PPV_ARGS(&shadowPassPipelineState)));
}

void D3D12RHI::WaitForPreviousFrame()
{
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

void D3D12RHI::RHICreateSynObject()
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



