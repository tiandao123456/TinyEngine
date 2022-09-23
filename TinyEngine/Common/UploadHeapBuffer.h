#pragma once
#include <conio.h>
#include <wrl.h>
#include <d3d12.h>
#include "..\d3dx12.h"

using Microsoft::WRL::ComPtr;

template<typename T>
class UploadHeapBuffer
{
protected:
    ComPtr<ID3D12Resource> uploadHeapBuffer;
    BYTE* mappedData = nullptr;
    UINT elementByteSize = 0;
    bool isConstantBuffer = false;

public:
    UploadHeapBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
        isConstantBuffer(isConstantBuffer)
    {
        elementByteSize = sizeof(T);

        // 如果是constant buffer，则每个元素（每个constant buffer）的大小与 256byte 对齐
        if (isConstantBuffer)
            elementByteSize = (sizeof(T) + 255) & ~255;

		//AllocConsole();
		//_cprintf("%d ", isConstantBuffer);

        auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto descSize = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementCount);
        device->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &descSize,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadHeapBuffer));

        uploadHeapBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    }

    UploadHeapBuffer(const UploadHeapBuffer& rhs) = delete;
    UploadHeapBuffer& operator=(const UploadHeapBuffer& rhs) = delete;
    ~UploadHeapBuffer()
    {
        if (uploadHeapBuffer != nullptr)
            uploadHeapBuffer->Unmap(0, nullptr);

        mappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return uploadHeapBuffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&mappedData[elementIndex * elementByteSize], &data, sizeof(T));
    }
};