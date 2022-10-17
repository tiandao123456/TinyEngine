#pragma once
#include "CommonEnum.h"

//’Î∂‘Ωªªª¡¥µƒ√Ë ˆ
class D3D12SCDesc
{
private:
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
public:
	D3D12SCDesc(UINT bufferCount, BufferUsageFormat bufferUsageFormat, SwapEffect swapEffect, UINT sampleDescCount, DataFormat dFormat)
	{
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));

		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Width = mClientWidth;
		swapChainDesc.Height = mClientHeight;
		swapChainDesc.SampleDesc.Count = sampleDescCount;

		switch (dFormat)
		{
		case R8G8B8A8:
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case D32:
			swapChainDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		default:
			break;
		}

		switch (bufferUsageFormat)
		{
		case Output:
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			break;
		case UnorderedAccess:
			swapChainDesc.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS;
			break;
		default:
			break;
		}

		switch (swapEffect)
		{
		case Discard:
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			break;
		case Sequential:
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
			break;
		case FlipSequential:
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			break;
		case FlipDiscard:
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			break;
		default:
			break;
		}
	}
	DXGI_SWAP_CHAIN_DESC1 &GetSwapChainDesc()
	{
		return swapChainDesc;
	}
};

