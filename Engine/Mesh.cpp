#include "pch.h"
#include "Mesh.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"
#include "TableDescriptorHeap.h"

Mesh::Mesh() : Object(OBJECT_TYPE::MESH) {}

Mesh::~Mesh() {}

void Mesh::Create(const vector<Vertex> &vertices, const vector<uint32> &indices) {
    CreateVertexBuffer(vertices);
    if (!indices.empty())
        CreateIndexBuffer(indices);
}

void Mesh::Render(uint32 instanceCount, uint32 idx) {
    if (_vecIndexBufferInfo.empty()) {
        RenderLines(instanceCount);
    } else {
        RenderIndexed(instanceCount, idx);
    }
}

void Mesh::RenderIndexed(uint32 instanceCount, uint32 idx) {
    GRAPHICS_CMD_LIST->IASetVertexBuffers(0, 1, &_vertexBufferView); // Slot: (0~15)
    GRAPHICS_CMD_LIST->IASetIndexBuffer(&_vecIndexBufferInfo[idx].indexBufferView);

    GRAPHICS_CMD_LIST->DrawIndexedInstanced(_vecIndexBufferInfo[idx].indexCount, instanceCount, 0, 0, 0);
}

void Mesh::RenderLines(uint32 instanceCount) {
    GRAPHICS_CMD_LIST->IASetVertexBuffers(0, 1, &_vertexBufferView);

    GRAPHICS_CMD_LIST->DrawInstanced(_vertexCount, instanceCount, 0, 0);
}

void Mesh::CreateVertexBuffer(const vector<Vertex> &vertices) {
    _vertexCount = static_cast<uint32>(vertices.size());
    uint32 bufferSize = _vertexCount * sizeof(Vertex);

    D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // default 힙으로 복사 추가?
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    DEVICE->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr, IID_PPV_ARGS(&_vertexBuffer));

    // Copy the triangle data to the vertex buffer.
    void *vertexDataBuffer = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    _vertexBuffer->Map(0, &readRange, &vertexDataBuffer);
    ::memcpy(vertexDataBuffer, vertices.data(), bufferSize);
    _vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex); // 정점 1개 크기
    _vertexBufferView.SizeInBytes = bufferSize;       // 버퍼의 크기
}

void Mesh::CreateIndexBuffer(const vector<uint32> &indices) {
    uint32 indexCount = static_cast<uint32>(indices.size());
    uint32 bufferSize = indexCount * sizeof(uint32);

    D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ComPtr<ID3D12Resource> indexBuffer;
    DEVICE->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr, IID_PPV_ARGS(&indexBuffer));

    void *indexDataBuffer = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    indexBuffer->Map(0, &readRange, &indexDataBuffer);
    ::memcpy(indexDataBuffer, indices.data(), bufferSize);
    indexBuffer->Unmap(0, nullptr);

    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView.SizeInBytes = bufferSize;

    IndexBufferInfo info = {indexBuffer, indexBufferView, DXGI_FORMAT_R32_UINT, indexCount};

    _vecIndexBufferInfo.push_back(info);
}
