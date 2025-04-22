#include "pch.h"
#include "Timer.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"

void Timer::Init() {
    ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&_frequency));
    ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&_prevCount)); // CPU 클럭

    // Timestamp Frequency 얻기
    GEngine->GetComputeCmdQueue()->GetCmdQueue()->GetTimestampFrequency(&_timestampFrequency);

    // Query Heap 생성
    D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    queryHeapDesc.Count = 2;
    queryHeapDesc.NodeMask = 0;
    DEVICE->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_queryHeap));

    // 쿼리 결과를 저장할 리소스 생성
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64) * 2);
    D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

    DEVICE->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                    nullptr, IID_PPV_ARGS(&_queryResultBuffer));
}

void Timer::Update() {
    uint64 currentCount;
    ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&currentCount));

    _deltaTime = (currentCount - _prevCount) / static_cast<float>(_frequency);
    _prevCount = currentCount;

    _frameCount++;
    _frameTime += _deltaTime;

    if (_frameTime > 1.f) {
        _fps = static_cast<uint32>(_frameCount / _frameTime);

        _frameTime = 0.f;
        _frameCount = 0;
    }
}

void Timer::Start(bool measureGPU) {
    _measureGPU = measureGPU;
    _startTimeCPU = std::chrono::high_resolution_clock::now();

    if (_measureGPU) {
        GRAPHICS_CMD_LIST->EndQuery(_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0); // Start
    }
}

void Timer::End() {
    if (_measureGPU) {
        GRAPHICS_CMD_LIST->EndQuery(_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1); // End

        // GPU 메모리에 저장된 쿼리 데이터를 CPU로 복사
        GRAPHICS_CMD_LIST->ResolveQueryData(_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2,
                                            _queryResultBuffer.Get(), 0);
    }

    _elapsedTimeCPU =
        std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - _startTimeCPU).count();

    if (_measureGPU) {
        // GPU 대기 후 읽기 (플러시 필요)
        GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();

        UINT64 *resultData = nullptr;
        D3D12_RANGE readRange = {0, sizeof(UINT64) * 2};
        _queryResultBuffer->Map(0, &readRange, reinterpret_cast<void **>(&resultData));

        _startTimestamp = resultData[0];
        _endTimestamp = resultData[1];
        _queryResultBuffer->Unmap(0, nullptr);

        _elapsedTimeGPU = (_endTimestamp - _startTimestamp) * 1000.0 / _timestampFrequency;

        cout << "GPU: " << _elapsedTimeGPU << " ms, ";
    }

    cout << "CPU: " << _elapsedTimeCPU << " ms" << std::endl;
}
