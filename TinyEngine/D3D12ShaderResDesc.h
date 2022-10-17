#pragma once
#include "CommonEnum.h"
class D3D12ShaderResDesc
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResDesc;
public:
	D3D12ShaderResDesc(){}
	const D3D12_SHADER_RESOURCE_VIEW_DESC &GetShaderResDesc(UINT shader4ComponentMapping, DXGI_FORMAT dxgiFormat, UINT detailedMip, UINT mipLevel, SrvDimension srvDimension)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Shader4ComponentMapping = shader4ComponentMapping;
		srvDesc.Format = dxgiFormat;
		srvDesc.Texture2D.MostDetailedMip = detailedMip;
		srvDesc.Texture2D.MipLevels = mipLevel;

		switch (srvDimension)
		{
		case SRVTexture2D:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			break;
		default:
			break;
		}
		shaderResDesc = srvDesc;
		return shaderResDesc;
	}
};

