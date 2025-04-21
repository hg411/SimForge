#pragma once

enum class OBJECT_TYPE : uint8 {
    NONE,
    SIMULATION_OBJECT, // PREFAB
    COMPONENT,
    GRAPHICS_MATERIAL,
    COMPUTE_MATERIAL,
    MESH,
    MESH_DATA,
    SHADER,
    TEXTURE,

    END
};

enum { OBJECT_TYPE_COUNT = static_cast<uint8>(OBJECT_TYPE::END) };

class Object {
  public:
    Object(OBJECT_TYPE type);
    virtual ~Object();

    void SetName(const wstring &name) { _name = name; }
    const wstring &GetName() { return _name; }

    OBJECT_TYPE GetType() { return _objectType; }

  protected:
    friend class Resources;
    virtual void Load(const wstring &path) {}
    virtual void Save(const wstring &path) {}

  protected:
    OBJECT_TYPE _objectType = OBJECT_TYPE::NONE;
    wstring _name;
};
