#pragma once
#include "UploadHeapBuffer.h"

template<typename T>
class UploadHeapConstantBuffer : public UploadHeapBuffer<T>
{
public:
    UploadHeapConstantBuffer(ID3D12Device* device, UINT elementCount) :
        UploadHeapBuffer<T>(device, elementCount, true)
    {

    }

    // ´´½¨CBV
    void CreateConstantBufferView(ID3D12Device* device, const CD3DX12_CPU_DESCRIPTOR_HANDLE& cbvHeapHandle, const UINT index)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        D3D12_GPU_VIRTUAL_ADDRESS startAddress = this->uploadHeapBuffer->GetGPUVirtualAddress();
        startAddress += index * this->elementByteSize;
        cbvDesc.BufferLocation = startAddress;
        cbvDesc.SizeInBytes = this->elementByteSize;
        device->CreateConstantBufferView(&cbvDesc, cbvHeapHandle);
    }
};