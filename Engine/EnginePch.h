#pragma once

// std::byte 사용하지 않음
#define _HAS_STD_BYTE 0

// 각종 include
#include <windows.h>
#include <tchar.h>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <list>
#include <map>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

#include "d3dx12.h"
#include <directxtk/SimpleMath.h>
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

// #include <DirectXTex/DirectXTex.h>
// #include <DirectXTex/DirectXTex.inl>
#include <DirectXTex.h>
#include <DirectXTexEXr.h> // EXR 형식 HDRI 읽기
//
// #include "FBX/fbxsdk.h"

// Imgui
// imgui ,SimpleMath 추가
#include <imgui.h>
#include "imgui_impl_dx12.h"
#include <imgui_impl_win32.h>

// 각종 lib
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

// #ifdef _DEBUG
// #pragma comment(lib, "DirectXTex\\DirectXTex_debug.lib")
// #else
// #pragma comment(lib, "DirectXTex\\DirectXTex.lib")
// #endif

// #ifdef _DEBUG
// #pragma comment(lib, "FBX\\debug\\libfbxsdk-md.lib")
// #pragma comment(lib, "FBX\\debug\\libxml2-md.lib")
// #pragma comment(lib, "FBX\\debug\\zlib-md.lib")
// #else
// #pragma comment(lib, "FBX\\release\\libfbxsdk-md.lib")
// #pragma comment(lib, "FBX\\release\\libxml2-md.lib")
// #pragma comment(lib, "FBX\\release\\zlib-md.lib")
// #endif

// 각종 typedef
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;
using Vec2 = DirectX::SimpleMath::Vector2;
using Vec3 = DirectX::SimpleMath::Vector3;
using Vec4 = DirectX::SimpleMath::Vector4;
using Matrix = DirectX::SimpleMath::Matrix;
using Quaternion = DirectX::SimpleMath::Quaternion;

enum class CBV_REGISTER : uint8 {
    b0,
    b1,
    b2,
    b3,
    b4,
    b5,
    b6,
    b7,
    b8,
    b9,

    END
};

enum class SRV_REGISTER : uint8 {
    t0 = static_cast<uint8>(CBV_REGISTER::END),
    t1,
    t2,
    t3,
    t4,
    t5,
    t6,
    t7,
    t8,
    t9,

    END
};

enum class UAV_REGISTER : uint8 {
    u0 = static_cast<uint8>(SRV_REGISTER::END),
    u1,
    u2,
    u3,
    u4,
    u5,
    u6,
    u7,
    u8,
    u9,

    END,
};

enum : uint8 {
    SWAP_CHAIN_BUFFER_COUNT = 2,
    CBV_REGISTER_COUNT = static_cast<uint8>(CBV_REGISTER::END),
    SRV_REGISTER_COUNT = static_cast<uint8>(SRV_REGISTER::END) - CBV_REGISTER_COUNT,
    CBV_SRV_REGISTER_COUNT = CBV_REGISTER_COUNT + SRV_REGISTER_COUNT,
    UAV_REGISTER_COUNT = static_cast<uint8>(UAV_REGISTER::END) - CBV_SRV_REGISTER_COUNT,
    TOTAL_REGISTER_COUNT = CBV_SRV_REGISTER_COUNT + UAV_REGISTER_COUNT,
};

enum class ROOT_PARAM_GRAPHICS : uint32 { CBV_GLOBAL = 0, CBV_TABLE = 1, SRV_TABLE = 2 };

enum class ROOT_PARAM_COMPUTE : uint32 { CBV_TABLE = 0, SRV_TABLE = 1, UAV_TABLE = 2 };

struct WindowInfo {
    HWND hwnd;     // 출력 윈도우
    int32 width;   // 너비
    int32 height;  // 높이
    bool windowed; // 창모드 or 전체화면
};

struct Vertex {
    Vertex() {}

    Vertex(Vec3 p, Vec2 u = Vec2(0.0f), Vec3 n = Vec3(0.0f), Vec3 t = Vec3(0.0f))
        : position(p), uv(u), normal(n), tangent(t) {}

    Vec3 position = Vec3(0.0f);
    Vec2 uv = Vec2(0.0f);
    Vec3 normal = Vec3(0.0f);
    Vec3 tangent = Vec3(0.0f);
};

#define DECLARE_SINGLE(type)                                                                                           \
  private:                                                                                                             \
    type() {}                                                                                                          \
    ~type() {}                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    static type *GetInstance() {                                                                                       \
        static type instance;                                                                                          \
        return &instance;                                                                                              \
    }

#define GET_SINGLE(type) type::GetInstance()

#define DEVICE GEngine->GetDevice()->GetDevice()
#define GRAPHICS_CMD_LIST GEngine->GetGraphicsCmdQueue()->GetGraphicsCmdList()
#define RESOURCE_CMD_LIST GEngine->GetGraphicsCmdQueue()->GetResourceCmdList()
#define COMPUTE_CMD_LIST GEngine->GetComputeCmdQueue()->GetComputeCmdList()

#define GRAPHICS_ROOT_SIGNATURE GEngine->GetRootSignature()->GetGraphicsRootSignature()
#define COMPUTE_ROOT_SIGNATURE GEngine->GetRootSignature()->GetComputeRootSignature()

#define INPUT GET_SINGLE(Input)
#define DELTA_TIME GET_SINGLE(Timer)->GetDeltaTime()

#define CONST_BUFFER(type) GEngine->GetConstantBuffer(type)


struct AnimFrameParams {
    Vec4 sacle;
    Vec4 rotation; // Quaternion
    Vec4 translation;
};

extern unique_ptr<class Engine> GEngine;

// Utils
wstring s2ws(const string &s);
string ws2s(const wstring &s);