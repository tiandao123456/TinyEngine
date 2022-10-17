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

	commandList->SetPipelineState(pipelineState["shadowPsoDesc"].Get());
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
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState["basePsoDesc"].Get()));

	RHIDrawSceneToShadow();
	//���ӿڵ���Ⱦ���ߵĹ�դ���׶�
	commandList->RSSetViewports(1, &baseViewport);
	commandList->RSSetScissorRects(1, &baseScissorRect);

	// ��Ⱦ��off-screen������base pass���л��
	// Change offscreen texture to be used as a a render target output.
	auto resourceBarrierForPostProcess = CD3DX12_RESOURCE_BARRIER::Transition(offScreenResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resourceBarrierForPostProcess);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandleForPostProcess(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandleForPostProcess.Offset(2, rtvDescriptorSize);
	// Clear the back buffer and depth buffer.
	DirectX::XMFLOAT4 fogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandleForPostProcess, (float*)&fogColor, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Specify the buffers we are going to render to.
	commandList->OMSetRenderTargets(1, &rtvHandleForPostProcess, true, &depthStencilView);

	//// ������̨���彫��ʹ����Ϊ��ȾĿ��
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	//CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	//commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	//// Record commands.
	//const float clearColor[] = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	//commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	//commandList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//auto resourceFromPresentToTarget = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	
	// ֪ͨ������������Ҫͬ������Դ�Ķ�η��� 
	// ����Դ���������ͬ������Դ���ϼ�������Դ��״̬ת��
	// GPU��ĳ��ALU�Ը���Դ������ֻ��������ĳ��ALU������ֻд������ʹ����Դ���ϾͿ��Ը���Դһ��״̬
	// ����Դ���ֻ��״̬ʱ������ĳЩALU���Խ��ж���Դ���в����ˣ�ֻдͬ��
	// commandList->ResourceBarrier(1, &resourceFromPresentToTarget);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->SetPipelineState(pipelineState["basePsoDesc"].Get());

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
	//auto resourceFromTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//// ������̨�������ڽ�������
	//commandList->ResourceBarrier(1, &resourceFromTargetToPresent);
	auto x = CD3DX12_RESOURCE_BARRIER::Transition(offScreenResource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &x);

	commandList->SetComputeRootSignature(postProcessRootSignature.Get());
	commandList->SetPipelineState(pipelineState["sobelPsoDesc"].Get());

	auto handleForPostProcess = postProcessHandleSaved;
	auto handleForPostProcessUav = postProcessHandleSaved;
	handleForPostProcess.Offset(2, cbvSrvUavDescriptorSize);
	handleForPostProcessUav.Offset(1, cbvSrvUavDescriptorSize);
	commandList->SetComputeRootDescriptorTable(0, handleForPostProcess);
	commandList->SetComputeRootDescriptorTable(2, handleForPostProcessUav);

	//ת����Դ״̬�Ӷ����ܹ������������
	auto resourceFromReadToAccess = CD3DX12_RESOURCE_BARRIER::Transition(outputResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(1, &resourceFromReadToAccess);

	// How many groups do we need to dispatch to cover image, where each
	// group covers 16x16 pixels.
	UINT numGroupsX = (UINT)ceilf(mClientWidth / 16.0f);
	UINT numGroupsY = (UINT)ceilf(mClientHeight / 16.0f);
	commandList->Dispatch(numGroupsX, numGroupsY, 1);

	auto resourceFromAccessToRead = CD3DX12_RESOURCE_BARRIER::Transition(outputResource.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &resourceFromAccessToRead);

	auto resourceFromPresentToTarget = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[currBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resourceFromPresentToTarget);

	CD3DX12_CPU_DESCRIPTOR_HANDLE currentBackBufferView(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currBackBuffer, rtvDescriptorSize);
	// Specify the buffers we are going to render to.
	commandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);

	commandList->SetGraphicsRootSignature(postProcessRootSignature.Get());
	commandList->SetPipelineState(pipelineState["compositePsoDesc"].Get());
	commandList->SetGraphicsRootDescriptorTable(0, handleForPostProcess);
	commandList->SetGraphicsRootDescriptorTable(1, postProcessHandleSaved);

	commandList->IASetVertexBuffers(0, 1, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->DrawInstanced(6, 1, 0, 0);

	// Indicate a state transition on the resource usage.
	auto y = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[currBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &y);

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
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * Pi, theNumOneCamera.aspect, 1.0f, 1000.0f);
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

	auto camera = SceneManage::GetInstance().GetCameraActor();
	auto theNumOneCamera = camera[camera.begin()->first];
	constMatrix.cameraLoc.x = theNumOneCamera.location[0];
	constMatrix.cameraLoc.y = theNumOneCamera.location[1];
	constMatrix.cameraLoc.z = theNumOneCamera.location[2];

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
	//ThrowIfFailed(dxgiSwapChain->Present(1, 0));

	ThrowIfFailed(dxgiSwapChain->Present(0, 0));
	currBackBuffer = (currBackBuffer + 1) % (swapChainBufferCount);

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
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState["basePsoDesc"].Get(), IID_PPV_ARGS(&commandList)));
}

void D3D12RHI::RHICreateSwapChain(UINT bufferCount, BufferUsageFormat bufferUsageFormat, SwapEffect swapEffect, UINT sampleDescCount, DataFormat dFormat)
{
	D3D12SCDesc scDesc(bufferCount, bufferUsageFormat, swapEffect, sampleDescCount, dFormat);
	HWND mhMainWnd = Win::GetInstance().mhMainWnd;
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),// ��������Ҫ������в���ˢ�£��������������һ��ָ��ָʾ����������ǰ��̨����Ľ���
		mhMainWnd, &scDesc.GetSwapChainDesc(), nullptr, nullptr, &swapChain));

	//�ô��ڽ�������Ӧalt-enter���У�������ϣ�
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&dxgiSwapChain));
	frameIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
}

ComPtr<ID3D12DescriptorHeap> D3D12RHI::RHICreateDescriptorHeap(HeapType heapType, UINT descriptorNums, UINT nodeMask, HeapFlag heapFlag)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	ZeroMemory(&heapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

	switch (heapType)
	{
	case RTV:
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		break;
	case DSV:
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		break;
	case CBVSRVUAV:
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		break;
	default:
		break;
	}
	switch (heapFlag)
	{
	case HeapNone:
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		break;
	case HeapVisible:
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		break;
	default:
		break;
	}
	heapDesc.NumDescriptors = descriptorNums;
	heapDesc.NodeMask = nodeMask;

	ComPtr<ID3D12DescriptorHeap> heap;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
	return heap;
}

//����rtv
void D3D12RHI::RHICreateRenderTarget(UINT renderTargetNums)
{
	// descriptor��ʾ��Դ���Դ��еĴ�ŵ�ַ������Դ�����Ƕ�������ȣ�
	// heaps���Ǵ��descriptor��һƬ�ڴ�
	// ������ȾĿ����ͼ����ȾĿ����ͼ����Ⱦ���ߵ�����Ŀ�������λ�ã���Ϊ��Ⱦ�������ղ����������Ļ�ϣ������������ȾĿ����ͼ�ϣ�
	// �����Ļ��ȡ��ȾĿ����ͼ�����ݲ�����
	rtvHeap = RHICreateDescriptorHeap(RTV, renderTargetNums + 1);

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

void D3D12RHI::RHICreateDepthStencil(DataFormat dFormat, UINT shadowMapWidth, UINT shadowMapHeight)
{
	//�������ģ���
	//���/ģ��Ѵ�СΪ2����Ϊ��һ��shadowMap
	dsvHeap = RHICreateDescriptorHeap(DSV, 2);
	//���������ģ�建��Ļ�����������ᵲס�Ȼ��Ƶ����壬�����Ǹ���ʵ�ʵ���Ȼ���

	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12CCRes createCommitedRes;
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = createCommitedRes.GetDepthStencilViewDesc(D32, Texture2D, None);
	D3D12_CLEAR_VALUE depthOptimizedClearValue = createCommitedRes.GetClearValue(D32, 1.0, 0);
	D3D12_RESOURCE_DESC resDesc = createCommitedRes.GetResDesc(ResTexture2D, 0, mClientWidth, mClientHeight, 1, 0, D32, 1, 0, LayoutUnknown, FlagAllowDepthStencil);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)));
	d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//shadowMap�����Դ,��shadow map�����dsv����
	shadowMapViewport = { 0.0f, 0.0f, (float)shadowMapWidth, (float)shadowMapHeight, 0.0f, 1.0f };
	shadowMapScissorRect = { 0, 0, (int)shadowMapWidth, (int)shadowMapHeight };
	depthStencilDesc = createCommitedRes.GetDepthStencilViewDesc(D24S8, Texture2D, None);
	depthOptimizedClearValue = createCommitedRes.GetClearValue(D24S8, 1.0, 0);
	D3D12_RESOURCE_DESC shadowResDesc = createCommitedRes.GetResDesc(ResTexture2D, 0, shadowMapWidth, shadowMapHeight, 1, 1, R24G8, 1, 0, LayoutUnknown, FlagAllowDepthStencil);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&shadowResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&shadowMapResource)));
	dsvHeapHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	shadowDsvHeapHandle = dsvHeapHandle;
	shadowDsvHeapHandle.Offset(1, dsvDescriptorSize);
	d3dDevice->CreateDepthStencilView(shadowMapResource.Get(), &depthStencilDesc, shadowDsvHeapHandle);

	D3D12_RESOURCE_DESC outputResDesc = createCommitedRes.GetResDesc(ResTexture2D, 0, mClientWidth, mClientHeight, 1, 1, R8G8B8A8, 1, 0, LayoutUnknown, FlagAllowUnorderedAccess);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&outputResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&outputResource)));

	D3D12_RESOURCE_DESC offScreenResDesc = createCommitedRes.GetResDesc(ResTexture2D, 0, mClientWidth, mClientHeight, 1, 1, R8G8B8A8, 1, 0, LayoutUnknown, FlagAllowRenderTarget);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&offScreenResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&offScreenResource)));
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> D3D12RHI::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return 
	{
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp, shadow 
	};
}

void D3D12RHI::RHICreateRootSignature(UINT rootParamNums, CD3DX12_ROOT_PARAMETER slotRootParameter[], ComPtr<ID3D12RootSignature> rootSignatureParam, UINT flag)
{
	auto staticSamplers = GetStaticSamplers();
	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(rootParamNums, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(rootSignatureParam.GetAddressOf())));

	if (flag)
		postProcessRootSignature = rootSignatureParam;
	else
		rootSignature = rootSignatureParam;
}

void D3D12RHI::RHICreatePostProcessRootDescriptorTable()
{
	CD3DX12_DESCRIPTOR_RANGE desRange[3];
	desRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	desRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	desRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &desRange[0]);
	slotRootParameter[1].InitAsDescriptorTable(1, &desRange[1]);
	slotRootParameter[2].InitAsDescriptorTable(1, &desRange[2]);

	RHICreateRootSignature(3, slotRootParameter, postProcessRootSignature, 1);

}

//�����������������ѵĹ�ϵ����������ʵ�������������ѵ��ӷ�Χ
//srv�����Ҫ����shader�У�������뵽���������У�descriptor table��
//Ȼ��cbvͬ��Ҳ���Էŵ����������У�Ҳ���Բ���ֱ�Ӵ��䵽shader��
void D3D12RHI::RHICreateRootDescriptorTable()
{
	CD3DX12_DESCRIPTOR_RANGE ranges[5];
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

	CD3DX12_ROOT_PARAMETER rootParameters[5];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);

	RHICreateRootSignature(5, rootParameters, rootSignature, 0);
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
	//���ݳ����е������������Ҫ��cbv��srv����
	const auto& staticMeshActor = SceneManage::GetInstance().GetStaticMeshActor();
	for (auto iter = staticMeshActor.begin(); iter != staticMeshActor.end(); iter++)
	{
		//cbv������
		cbvNums += iter->second.worldMatrix.size();
		srvNums += iter->second.worldMatrix.size() * 2;
	}
	//����shadow map
	srvNums += 1;

	// ���������е�cbv��srv��uav����
	// 1Ϊһ�������ṹ��3Ϊ�����Ե�����Ҫ����Դ
	cbvSrvUavHeap = RHICreateDescriptorHeap(CBVSRVUAV, cbvNums + srvNums + 1 + 3, 0, HeapVisible);

	worldMatrixBuffer = std::make_unique<UploadHeapConstantBuffer<WorldMatrix>>(d3dDevice.Get(), cbvNums);
	constMatrixBuffer = std::make_unique<UploadHeapConstantBuffer<ConstMatrix>>(d3dDevice.Get(), 1);
	cbvSrvUavHeapHandle = cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	cbvSrvUavHeapHandleForGpu = cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();

	//�������cb����������Ӧģ�͵��������
	for (auto i = 0; i < cbvNums; i++)
	{
		worldMatrixBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, i);
		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);
	}
	//����һ��cb����������ӦConstMatirx
	constMatrixBuffer->CreateConstantBufferView(d3dDevice.Get(), cbvSrvUavHeapHandle, 0);

	LoadTexture();
	auto diffuseTexBrick = textures["brickDiffuse"]->resource;
	auto diffuseTexTile = textures["tileDiffuse"]->resource;
	auto normalTexBrick = textures["brickNormal"]->resource;
	auto normalTexTile = textures["tileNormal"]->resource;

	D3D12ShaderResDesc shaderResDesc;
	//�������sr��������
	for (auto i = 0; i < (srvNums - 1) / 4; i++)
	{
		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);
		d3dDevice->CreateShaderResourceView(diffuseTexBrick.Get(), 
			&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, diffuseTexBrick->GetDesc().Format, 0, -1, SRVTexture2D), cbvSrvUavHeapHandle);

		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);
		d3dDevice->CreateShaderResourceView(normalTexBrick.Get(),
			&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, normalTexBrick->GetDesc().Format, 0, -1, SRVTexture2D), cbvSrvUavHeapHandle);

		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);
		d3dDevice->CreateShaderResourceView(diffuseTexTile.Get(),
			&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, diffuseTexTile->GetDesc().Format, 0, -1, SRVTexture2D), cbvSrvUavHeapHandle);

		cbvSrvUavHeapHandle.Offset(1, cbvSrvUavDescriptorSize);
		cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);
		d3dDevice->CreateShaderResourceView(normalTexTile.Get(),
			&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, normalTexTile->GetDesc().Format, 0, -1, SRVTexture2D), cbvSrvUavHeapHandle);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapHandle = cbvSrvUavHeapHandle;
	shadowMapHandle.Offset(1, cbvSrvUavDescriptorSize);
	cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);

	d3dDevice->CreateShaderResourceView(shadowMapResource.Get(),
		&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 0, 1, SRVTexture2D), shadowMapHandle);

	auto postProcessHandle = shadowMapHandle;
	postProcessHandle.Offset(1, cbvSrvUavDescriptorSize);
	cbvSrvUavHeapHandleForGpu.Offset(1, cbvSrvUavDescriptorSize);
	postProcessHandleSaved = cbvSrvUavHeapHandleForGpu;
	d3dDevice->CreateShaderResourceView(outputResource.Get(),
		&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1, SRVTexture2D), postProcessHandle);

	postProcessHandle.Offset(1, cbvSrvUavDescriptorSize);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	d3dDevice->CreateUnorderedAccessView(outputResource.Get(), nullptr, &uavDesc, postProcessHandle);

	//render target����Դ
	postProcessHandle.Offset(1, cbvSrvUavDescriptorSize);
	d3dDevice->CreateShaderResourceView(offScreenResource.Get(),
		&shaderResDesc.GetShaderResDesc(D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1, SRVTexture2D), postProcessHandle);

	//��render target���ڳ���soble֮��ı�Ե���ͼ��֮����base pass�����ں�
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(2, rtvDescriptorSize);
	d3dDevice->CreateRenderTargetView(offScreenResource.Get(), nullptr, rtvHandle);
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
	//���嶥������벼��
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		//����λ�ô�0ƫ�ƴ���ʼ��������ɫ�ӵ�12���ֽڣ�x,y,z����ʼ
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	//���������������ܹ��������hlsl�ļ�
	shaders["MainShaderVS"] = d3dUtil::CompileShader(L"Shaders\\MainShader.hlsl", nullptr, "VS", "vs_5_1");
	shaders["MainShaderPS"] = d3dUtil::CompileShader(L"Shaders\\MainShader.hlsl", nullptr, "PS", "ps_5_1");
	shaders["GenerateShadowVS"] = d3dUtil::CompileShader(L"Shaders\\GenerateShadow.hlsl", nullptr, "VS", "vs_5_1");
	shaders["GenerateShadowPS"] = d3dUtil::CompileShader(L"Shaders\\GenerateShadow.hlsl", nullptr, "PS", "ps_5_1");
	shaders["compositeVS"] = d3dUtil::CompileShader(L"Shaders\\Composite.hlsl", nullptr, "VS", "vs_5_1");
	shaders["compositePS"] = d3dUtil::CompileShader(L"Shaders\\Composite.hlsl", nullptr, "PS", "ps_5_1");
	shaders["sobelCS"] = d3dUtil::CompileShader(L"Shaders\\Sobel.hlsl", nullptr, "SobelCS", "cs_5_1");

	//base pass pipeline state object desc
	D3D12PSODesc basePsoDesc(inputElementDescs, _countof(inputElementDescs),
		rootSignature, shaders, "MainShaderVS", "MainShaderPS",CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT), 
		CD3DX12_BLEND_DESC(D3D12_DEFAULT),CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT), UINT_MAX,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
	d3dDevice->CreateGraphicsPipelineState(&basePsoDesc.GetPSODesc(), IID_PPV_ARGS(&pipelineState["basePsoDesc"]));

	//shadow pass pipeline state object desc
	CD3DX12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.DepthBias = 100000;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 1.0f;
	D3D12PSODesc shadowPsoDesc(inputElementDescs, _countof(inputElementDescs),
		rootSignature, shaders, "GenerateShadowVS", "GenerateShadowPS", rasterizerDesc,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT), CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT), UINT_MAX,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 0,
		DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_UNKNOWN);
	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc.GetPSODesc(), IID_PPV_ARGS(&pipelineState["shadowPsoDesc"])));

	//composite pipeline state object desc
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	D3D12PSODesc compositePsoDesc(inputElementDescs, _countof(inputElementDescs),
		postProcessRootSignature, shaders, "compositeVS", "compositePS", CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		CD3DX12_BLEND_DESC(D3D12_DEFAULT), depthStencilDesc, UINT_MAX, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM, 1, 0, DXGI_FORMAT_D24_UNORM_S8_UINT);
	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&compositePsoDesc.GetPSODesc(), IID_PPV_ARGS(&pipelineState["compositePsoDesc"])));

	D3D12_COMPUTE_PIPELINE_STATE_DESC sobelPsoDesc = {};
	sobelPsoDesc.pRootSignature = postProcessRootSignature.Get();
	sobelPsoDesc.CS =
	{
		reinterpret_cast<BYTE*>(shaders["sobelCS"]->GetBufferPointer()),
		shaders["sobelCS"]->GetBufferSize()
	};
	sobelPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateComputePipelineState(&sobelPsoDesc, IID_PPV_ARGS(&pipelineState["sobelPsoDesc"])));
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



