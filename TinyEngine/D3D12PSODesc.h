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
		UINT count,
		//默认值
		UINT quality = 0,
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT rtvFormat = DXGI_FORMAT_UNKNOWN
	)
	{
		//在给pipelineStateDesc赋值的时候需要先把它清空
		ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
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
		//描述混和状态
		pipelineStateDesc.BlendState = blendDesc;
		pipelineStateDesc.DepthStencilState = depthStencilDesc;//深度模板状态
		pipelineStateDesc.SampleMask = sampleMask;
		//解释集合或外壳着色器输入图元，此处定义图元为三角形
		pipelineStateDesc.PrimitiveTopologyType = primitiveTopologyType;
		pipelineStateDesc.NumRenderTargets = numRenderTargets;
		pipelineStateDesc.RTVFormats[0] =dxgiFormat;
		//描述资源的多重采样，此处设置为1即不进行多重采样
		pipelineStateDesc.SampleDesc.Count = count;

		pipelineStateDesc.SampleDesc.Quality = quality;
		pipelineStateDesc.DSVFormat = dsvFormat;
		pipelineStateDesc.RTVFormats[0] = rtvFormat;
	}
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC &GetPSODesc()
	{
		return pipelineStateDesc;
	}
};

