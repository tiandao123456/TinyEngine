#pragma once
#include "CommonEnum.h"
class D3D12CCRes
{
public:
	D3D12CCRes(){}
	D3D12_DEPTH_STENCIL_VIEW_DESC GetDepthStencilViewDesc(DataFormat dsFormat, ViewDimension viewDimension, Flag flag, UINT mipSlice = 0)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));

		switch (dsFormat)
		{
		case R8G8B8A8:
			depthStencilDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case D32:
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		case D24S8:
			depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			break;
		}

		switch (viewDimension)
		{
		case Texture2D:
			depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			break;
		default:
			break;
		}

		switch (flag)
		{
		case None:
			depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
			break;
		case DSVReadOnlyDepth:
			break;
		case DSVReadOnlyStencil:
			break;
		default:
			break;
		}
		depthStencilDesc.Texture2D.MipSlice = mipSlice;

		return depthStencilDesc;
	}
	D3D12_CLEAR_VALUE GetClearValue(DataFormat clearValue, FLOAT depth, UINT8 stencil)
	{
		D3D12_CLEAR_VALUE depthOptimizedClearValue;
		ZeroMemory(&depthOptimizedClearValue, sizeof(D3D12_CLEAR_VALUE));

		switch (clearValue)
		{
		case R8G8B8A8:
			depthOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case D32:
			depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		case D24S8:
			depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			break;
		}
		depthOptimizedClearValue.DepthStencil.Depth = depth;
		depthOptimizedClearValue.DepthStencil.Stencil = stencil;

		return depthOptimizedClearValue;
	}

	D3D12_RESOURCE_DESC GetResDesc(ResDimension dimension, UINT64 alignment, UINT width, 
		UINT height, UINT16 depthOrArraySize, UINT16 mipLevels,DataFormat resFormat, 
		UINT count, UINT quality, TexLayout layout, ResFlag resFlag)
	{
		D3D12_RESOURCE_DESC resourceDesc;
		ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));

		resourceDesc.Alignment = alignment;
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = depthOrArraySize;
		resourceDesc.MipLevels = mipLevels;
		resourceDesc.SampleDesc.Count = count;
		resourceDesc.SampleDesc.Quality = quality;

		switch (dimension)
		{
		case ResTexture2D:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		default:
			break;
		}

		switch (resFormat)
		{
		case R8G8B8A8:
			resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case D32:
			resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		case R24G8:
			resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			break;
		default:
			break;
		}

		switch (layout)
		{
		case LayoutUnknown:
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			break;
		default:
			break;
		}

		switch (resFlag)
		{
		case ResNone:
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			break;
		case FlagAllowDepthStencil:
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			break;
		case FlagAllowUnorderedAccess:
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			break;
		case FlagAllowRenderTarget:
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			break;
		default:
			break;
		}
		return resourceDesc;
	}
};

