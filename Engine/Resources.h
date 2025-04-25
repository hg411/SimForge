#pragma once

#include "SimulationObject.h"

class Mesh;
class GraphicsMaterial;
class ComputeMaterial;
class Texture;

class Resources {
    DECLARE_SINGLE(Resources)

  public:
    template <typename T> shared_ptr<T> Load(const wstring &key, const wstring &path);
    template <typename T, typename... Args> shared_ptr<T> Load(const wstring &key, const wstring &path, Args &&...args);
    template <typename T> bool Add(const wstring &key, shared_ptr<T> object);
    template <typename T> shared_ptr<T> Get(const wstring &key);
    template <typename T> bool Remove(const wstring &key);
    template <typename T> OBJECT_TYPE GetObjectType();

    // Shader
    void CreateLineShader();
    void CreateWireFrameShader();

    // Mesh
    shared_ptr<Mesh> LoadRectangleLineMesh(const float width = 1.0f, const float height = 1.0f);
    shared_ptr<Mesh> LoadCubeLineMesh(const float width = 1.0f, const float height = 1.0f, const float depth = 1.0f);
    shared_ptr<Mesh> LoadCubeMesh(const float width = 1.0f, const float height = 1.0f, const float depth = 1.0f);

  private:
    using KeyObjMap = std::map<wstring /*key*/, shared_ptr<Object>>;
    array<KeyObjMap, OBJECT_TYPE_COUNT> _resources;
};

template <typename T> inline shared_ptr<T> Resources::Load(const wstring &key, const wstring &path) {
    OBJECT_TYPE objectType = GetObjectType<T>();
    KeyObjMap &keyObjMap = _resources[static_cast<uint8>(objectType)];

    auto findIt = keyObjMap.find(key);
    if (findIt != keyObjMap.end())
        return static_pointer_cast<T>(findIt->second);

    shared_ptr<T> object = make_shared<T>();
    object->Load(path);
    keyObjMap[key] = object;

    return object;
}

template <typename T, typename... Args>
shared_ptr<T> Resources::Load(const wstring &key, const wstring &path, Args &&...args) {
    OBJECT_TYPE objectType = GetObjectType<T>();
    KeyObjMap &keyObjMap = _resources[static_cast<uint8>(objectType)];

    // 이미 로드된 리소스인지 확인 (중복 로딩 방지)
    auto findIt = keyObjMap.find(key);
    if (findIt != keyObjMap.end())
        return static_pointer_cast<T>(findIt->second);

    // 새로운 리소스 생성 및 로드
    shared_ptr<T> object = make_shared<T>();
    object->Load(path, std::forward<Args>(args)...);
    keyObjMap[key] = object;

    return object;
}

template <typename T> bool Resources::Add(const wstring &key, shared_ptr<T> object) {
    OBJECT_TYPE objectType = GetObjectType<T>();
    KeyObjMap &keyObjMap = _resources[static_cast<uint8>(objectType)];

    auto findIt = keyObjMap.find(key);
    if (findIt != keyObjMap.end())
        return false;

    keyObjMap[key] = object;

    return true;
}

template <typename T> shared_ptr<T> Resources::Get(const wstring &key) {
    OBJECT_TYPE objectType = GetObjectType<T>();
    KeyObjMap &keyObjMap = _resources[static_cast<uint8>(objectType)];

    auto findIt = keyObjMap.find(key);
    if (findIt != keyObjMap.end())
        return static_pointer_cast<T>(findIt->second);

    return nullptr;
}

template <typename T> bool Resources::Remove(const wstring &key) {
    OBJECT_TYPE objectType = GetObjectType<T>();
    KeyObjMap &keyObjMap = _resources[static_cast<uint8>(objectType)];

    auto findIt = keyObjMap.find(key);
    if (findIt != keyObjMap.end()) {
        keyObjMap.erase(findIt); // 해당 리소스 제거
        return true;
    }

    return false; // 존재하지 않음
}

// 고급 템플릿
template <typename T> inline OBJECT_TYPE Resources::GetObjectType() {
    if (std::is_same_v<T, SimulationObject>)
        return OBJECT_TYPE::SIMULATION_OBJECT;
    else if (std::is_same_v<T, GraphicsMaterial>)
        return OBJECT_TYPE::GRAPHICS_MATERIAL;
    else if (std::is_same_v<T, ComputeMaterial>)
        return OBJECT_TYPE::COMPUTE_MATERIAL;
    else if (std::is_same_v<T, Mesh>)
        return OBJECT_TYPE::MESH;
    else if (std::is_same_v<T, Texture>)
        return OBJECT_TYPE::TEXTURE;
    else if (std::is_convertible_v<T, Component>)
        return OBJECT_TYPE::COMPONENT;
    else
        return OBJECT_TYPE::NONE;
}