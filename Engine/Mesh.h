#pragma once

#include "Object.h"

struct IndexBufferInfo {
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    DXGI_FORMAT indexFormat;
    uint32 indexCount;
};

class Mesh : public Object {
  public:
    Mesh();
    virtual ~Mesh();

    void Create(const vector<Vertex> &vertices, const vector<uint32> &indices = {});
    void Render(uint32 instanceCount = 1, uint32 idx = 0);
    void RenderIndexed(uint32 instanceCount, uint32 idx);
    void RenderLines(uint32 instanceCount);

  private:
    void CreateVertexBuffer(const vector<Vertex> &vertices);
    void CreateIndexBuffer(const vector<uint32> &indices);

  public:
    uint32 GetSubsetCount() { return static_cast<uint32>(_vecIndexBufferInfo.size()); }

  private:
    ComPtr<ID3D12Resource> _vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};
    uint32 _vertexCount = 0;

    vector<IndexBufferInfo> _vecIndexBufferInfo;
};
