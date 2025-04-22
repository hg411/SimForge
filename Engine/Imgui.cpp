#include "pch.h"
#include "Imgui.h"
#include "Engine.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandQueue.h"
#include "Simulation.h"

Imgui::Imgui() {}

Imgui::~Imgui() {}

void Imgui::Init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    _frameCount = static_cast<int>(SWAP_CHAIN_BUFFER_COUNT);

    // Descriptor Heap for ImGui
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_srvHeap));

    // DX12 + Win32 backend init
    ImGui_ImplWin32_Init(GEngine->GetWindowInfo().hwnd);
    ImGui_ImplDX12_Init(DEVICE.Get(), _frameCount, DXGI_FORMAT_R8G8B8A8_UNORM, _srvHeap.Get(),
                        _srvHeap->GetCPUDescriptorHandleForHeapStart(),
                        _srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void Imgui::Update(shared_ptr<Simulation> simulation) {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
     ImGui::Begin("Scene Control");

    // ImGui가 측정해주는 Framerate 출력
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    // ImGui 위치 고정
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

    ImGui::End();

    simulation->BuildUI();
}

void Imgui::Render() {
    ID3D12DescriptorHeap *heaps[] = {_srvHeap.Get()};
    GRAPHICS_CMD_LIST->SetDescriptorHeaps(1, heaps);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GRAPHICS_CMD_LIST.Get());
}
