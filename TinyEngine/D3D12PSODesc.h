#pragma once

#include <unordered_map>
using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3D12PSODesc
{
private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;

public:
	D3D12PSODesc(D3D12_INPUT_ELEMENT_DESC inputElementDescs[], UINT nums,
		ComPtr<ID3D12RootSignature> rootSignature,
		std::unordered_map<std::string, ComPtr<ID3DBlob>> shaders,
		std::string strVS,
		std::string strPS,
		CD3DX12_RASTERIZER_DESC rasterizerDesc,
		CD3DX12_BLEND_DESC blendDesc,
		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc,
		UINT sampleMask,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType,
		UINT numRenderTargets,
		DXGI_FORMAT dxgiFormat,
		UINT count
	)
	{
		pipelineStateDesc.InputLayout = { inputElementDescs, nums };
		pipelineStateDesc.pRootSignature = rootSignature.Get();
		pipelineStateDesc.VS =
		{
			reinterpret_cast<BYTE*>(shaders[strVS]->GetBufferPointer()),
			shaders[strVS]->GetBufferSize()
		};
		pipelineStateDesc.PS =
		{
			reinterpret_cast<BYTE*>(shaders[strPS]->GetBufferPointer()),
			shaders[strPS]->GetBufferSize()
		};
		pipelineStateDesc.RasterizerState = rasterizerDesc;
		//�������״̬
		pipelineStateDesc.BlendState = blendDesc;
		pipelineStateDesc.DepthStencilState = depthStencilDesc;//���ģ��״̬
		pipelineStateDesc.SampleMask = sampleMask;
		//���ͼ��ϻ������ɫ������ͼԪ���˴�����ͼԪΪ������
		pipelineStateDesc.PrimitiveTopologyType = primitiveTopologyType;
		pipelineStateDesc.NumRenderTargets = numRenderTargets;
		pipelineStateDesc.RTVFormats[0] =dxgiFormat;
		//������Դ�Ķ��ز������˴�����Ϊ1�������ж��ز���
		pipelineStateDesc.SampleDesc.Count = count;
	}
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC &GetPSODesc()
	{
		return pipelineStateDesc;
	}
};

