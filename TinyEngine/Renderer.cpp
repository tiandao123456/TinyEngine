#include "stdafx.h"
#include "Renderer.h"

void Renderer::Init()
{
	UINT platform = 0;
#if Windows == platform
	//����D3D12RHI����ת�ƶ�ռʽָ��Ȩ��
	std::unique_ptr<D3D12RHI> d3d12RHI(new D3D12RHI());
	rhiBase = std::move(d3d12RHI);

	rhiBase->RHIStartDebug();
	rhiBase->RHIEnumAdapter();
	rhiBase->RHICreateCommandObjects();

	rhiBase->RHICreateSwapChain(2, Output, FlipDiscard, 1, R8G8B8A8);
	rhiBase->RHICreateRenderTarget(2);
	rhiBase->RHICreateDepthStencil(D32, 8192, 8192);
	rhiBase->RHICreateRootDescriptorTable();
	rhiBase->RHICreatePostProcessRootDescriptorTable();
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
	//�ڴ˴����������vp�����Լ�vpt����
	//�������RHI���и��£�������api(directx,vulkan,opengl)
	//�ڵײ������Щ������Ϣ�������׸���vp������߼�
	//����directx�����ֳɵĺ������Դ˴�Ĭ�����ⲿ���������˾���
	//GenerateMatrix();

	rhiBase->RHIUpdate();
	rhiBase->RHIRender();
}