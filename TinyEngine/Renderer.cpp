#include "stdafx.h"
#include "Renderer.h"

void Renderer::Init()
{
	UINT platform = 0;
#if Windows == platform
	//创建D3D12RHI对象转移独占式指针权限
	std::unique_ptr<D3D12RHI> d3d12RHI(new D3D12RHI());
	rhiBase = std::move(d3d12RHI);

	rhiBase->RHIStartDebug();
	rhiBase->RHIEnumAdapter();
	rhiBase->RHICreateCommandObjects();
	rhiBase->RHICreateSwapChain(2, R8G8B8A8);
	rhiBase->RHICreateRenderTarget(2);

	rhiBase->RHICreateDepthStencil(D32, 2048, 2048);
	rhiBase->RHICreateRootDescriptorTable();
	rhiBase->RHICreateConstBufferAndShaderResource();
	rhiBase->RHISetVertexAndIndexBuffer();
	rhiBase->RHICreatePipeLineState();
	rhiBase->RHICreateSynObject();
#endif

#if Mac == platform
#endif

#if Android == platform
#endif

#if Ios == platform
#endif 
}

void Renderer::RenderTick()
{
	rhiBase->RHIUpdate();
	rhiBase->RHIRender();
}