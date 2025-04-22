#include "pch.h"
#include "SwapChain.h"

void SwapChain::Init(const WindowInfo &windowInfo, ComPtr<ID3D12Device> device,
                     ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue) {
    CreateSwapChain(windowInfo, dxgi, cmdQueue);
}

void SwapChain::Present() { _swapChain->Present(1, 0); }

void SwapChain::SwapIndex() { _backBufferIndex = (_backBufferIndex + 1) % SWAP_CHAIN_BUFFER_COUNT; }

void SwapChain::CreateSwapChain(const WindowInfo &windowInfo, ComPtr<IDXGIFactory> dxgi,
                                ComPtr<ID3D12CommandQueue> cmdQueue) {
    _swapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = static_cast<uint32>(windowInfo.width);   // ������ �ػ� �ʺ�
    sd.BufferDesc.Height = static_cast<uint32>(windowInfo.height); // ������ �ػ� ����
    sd.BufferDesc.RefreshRate.Numerator = 60;                      // ȭ�� ���� ����
    sd.BufferDesc.RefreshRate.Denominator = 1;                     // ȭ�� ���� ����
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             // ������ ���÷��� ����
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = 1; // ��Ƽ ���ø� OFF
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // �ĸ� ���ۿ� �������� ��
    sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT;         // ����+�ĸ� ����
    sd.OutputWindow = windowInfo.hwnd;
    sd.Windowed = windowInfo.windowed;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ���� �ĸ� ���� ��ü �� ���� ������ ���� ����
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    dxgi->CreateSwapChain(cmdQueue.Get(), &sd, &_swapChain);
}
