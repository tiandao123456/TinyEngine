#pragma once
#include "RHIBase.h"
#include "D3D12RHI.h"

class Renderer
{
private:
	//void EnumAdapter();
	//void CreateCommandObjects();
	//void CreateSwapChain();
	//void CreateRtvAndDsvDescriptorHeap();
	//void LoadTexture();
	//void CreateRootSignature();
	//void CreatePipelineState();
	//void CreateVertexAndIndexBuffer();
	//void CreateCbvSrvUavDescriptor();
	//void CreateDepthStencil();
	//void CreateSynObject();
	//void WaitForPreviousFrame();
	//void PopulateCommandList();
	//void CalculateViewProj();
	//void CreateShadowMap();
	//void CalculateShadowTransform();
	//void DrawSceneToShadow();

	//void OnRender();
	//void OnUpdate();
	//void OnDestroy();

private:
	std::unique_ptr<RHIBase> rhiBase = nullptr;

public:
	//外部调用接口
	void Init();
	void RenderTick();
};

