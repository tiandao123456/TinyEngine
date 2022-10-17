#pragma once
#include "CommonEnum.h"

class RHIBase
{
public:
	virtual void RHIStartDebug() {};
	virtual void RHIEnumAdapter() {};
	virtual void RHICreateCommandObjects() {};
	virtual void RHICreateSwapChain(UINT bufferCount, BufferUsageFormat bufferUsageFormat, SwapEffect swapEffect, UINT sampleDescCount, DataFormat dFormat) {};
	virtual void RHICreateRenderTarget(UINT renderTargetNums) {};
	//�������ģ�建�壬Ĭ������
	virtual void RHICreateDepthStencil(DataFormat dFormat, UINT shadowMapWidth, UINT shadowMapHeight) {};
	virtual void RHICreateRootDescriptorTable() {};
	virtual void RHICreateConstBufferAndShaderResource() {};
	virtual void RHISetVertexAndIndexBuffer() {};
	virtual void RHICreatePipeLineState() {};
	virtual void RHICreateSynObject() {};
	virtual void RHIUpdate() {};
	virtual void RHIRender() {};
	virtual void RHIPopulateCommandList() {};
	virtual void RHIDrawSceneToShadow() {};
	virtual void RHICreatePostProcessRootDescriptorTable() {};
};

