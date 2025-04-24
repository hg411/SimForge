#pragma once

#include "Simulation.h"

class Shader;
class ConstantBuffer;
class StructuredBuffer;

struct SPH2DFluidParams {
    uint32 maxParticles = 0;
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    float pressureCoeff = 0.0f;
    float nearPressureCoeff = 0.0f;
    float viscosity = 0.0f;
    float density0 = 0.0f;
    float mass = 0.0f;
    float smoothingLength = 0.0f;

    Vec3 boxCenter = {};
    float boxWidth = 0.0f;
    float boxHeight = 0.0f;
    float boxDepth = 0.0f;

    float cellSize = 0.0f;
    uint32 hashCount = 0;
    Vec3 gridOrigin = {};

    uint32 addCount = 0;
    float radius = 0.0f;
    float surfaceCoeff = 0.0f;
    float padding = 0.0f;
};

class SPH2DFluid : public Simulation {
  public:
    struct BitonicSortConsts {
        uint32 k;
        uint32 j;
    };

    struct CellRange {
        uint32 startIndex;
        uint32 endIndex;
    };

  public:
    SPH2DFluid();
    virtual ~SPH2DFluid();

    void Init() override;

    void Update() override;
    void FinalUpdate() override;
    void Render() override;

  private:
    void InitShaders() override;
    void InitConstantBuffers() override;
    void InitStructuredBuffers() override;
    void InitSimulationObjects() override;

    void BuildUI() override;

    void PushSimulationParams();
    void ActivateParticles();
    void HashingParticles();
    void SortParticles();
    void ComputeCellRange();
    void ComputeDensity();
    void PredictPositionVelocity();
    void IterativeEOS(uint32 iterationCount);
    void IterativeEOS1();
    void IterativeEOS2();
    void IterativeEOS3();
    void FinalEOS();
    void AnimateParticles();

  private:
    // Constant Buffer
    shared_ptr<ConstantBuffer> _simulationParamsCB;
    vector<shared_ptr<ConstantBuffer>> _bitonicSortCBs;

    // Structured Buffer
    shared_ptr<StructuredBuffer> _positionBuffer;
    shared_ptr<StructuredBuffer> _velocityBuffer;
    shared_ptr<StructuredBuffer> _forceBuffer;
    shared_ptr<StructuredBuffer> _densityBuffer;
    shared_ptr<StructuredBuffer> _pressureBuffer;
    shared_ptr<StructuredBuffer> _nearPressureBuffer;
    shared_ptr<StructuredBuffer> _aliveBuffer;
    shared_ptr<StructuredBuffer> _hashBuffer;
    shared_ptr<StructuredBuffer> _cellRangeBuffer;
    shared_ptr<StructuredBuffer> _predPositionBuffer;
    shared_ptr<StructuredBuffer> _predVelocityBuffer;

    // Shaders
    shared_ptr<Shader> _activateShader;
    shared_ptr<Shader> _hashShader;
    shared_ptr<Shader> _bitonicSortShader;
    shared_ptr<Shader> _cellRangeShader;
    shared_ptr<Shader> _densityShader;
    shared_ptr<Shader> _predictShader;
    shared_ptr<Shader> _iterativeEOS1Shader;
    shared_ptr<Shader> _iterativeEOS2Shader;
    shared_ptr<Shader> _iterativeEOS3Shader;
    shared_ptr<Shader> _finalEOSShader;
    shared_ptr<Shader> _animate1Shader;
    shared_ptr<Shader> _animate2Shader;
    shared_ptr<Shader> _animate3Shader;

    shared_ptr<Shader> _particleRenderShader;

    uint32 _maxParticles = 4096;
    uint32 _threadGroupCountX = 0;
    float _numThreadsX = 64.0f;
    float _deltaTime = 0.006f;
    float _radius = 1.0f / 128.0f; // dx = radius * 2.0f
    float _pressureCoeff = 1.0f;
    float _nearPressureCoeff = 0.2f;
    float _viscosity = 0.0005f;
    float _density0 = 1000.0f;
    float _smoothingLength = 2.6f * _radius * 2.0f; // smoothing length = dx * 1.5f
    float _cellSize = _smoothingLength * 1.1f;      //
    float _mass = 0.35f;                            // mass = dx * dx * density0
    uint32 _hashCount = 8192 * 4;
    SPH2DFluidParams _sph2DFluidParams;

    // Bounding Box
    Vec3 _boxCenter = Vec3(0.0f, 0.0f, 1.0f);
    float _boxWidth = 3.4f;
    float _boxHeight = 1.7f;

    // 프레임고정
    float _accumulatedTime = 0.0f;
    float _timeStep = 1.0f / 100.0f;
};
