#pragma once

#include <d3d11.h>

// ============================================================================
// DX11 Compute Shader Particle System
// GPU-based particle simulation using compute shaders.
// Particles are updated on the GPU via Dispatch() and rendered as point
// sprites via the existing DX11 draw pipeline.
// ============================================================================

struct GPUParticle {
  float pos[3];
  float vel[3];
  float life;
  float maxLife;
  float size;
  float color[4]; // RGBA
};

class CDX11ComputeParticles {
public:
  CDX11ComputeParticles();
  ~CDX11ComputeParticles();

  bool Initialize(ID3D11Device *pDevice, ID3D11DeviceContext *pContext,
                  int maxParticles = 65536);
  void Shutdown();
  bool IsInitialized() const { return m_bInitialized; }

  // Emit new particles at position with velocity spread
  void Emit(float x, float y, float z, int count, float spread = 1.0f);

  // Update all particles (GPU compute dispatch)
  void Update(float deltaTime);

  // Render particles as point sprites (binds SRV + draws)
  void Render();

  void SetGravity(float gx, float gy, float gz);

private:
  bool CreateShaders();
  bool CreateBuffers();

  ID3D11Device *m_pDevice;
  ID3D11DeviceContext *m_pContext;
  bool m_bInitialized;
  int m_iMaxParticles;
  int m_iActiveParticles;

  // Compute shader for particle update
  ID3D11ComputeShader *m_pUpdateCS;

  // Particle buffer (structured buffer with UAV + SRV)
  ID3D11Buffer *m_pParticleBuffer;
  ID3D11UnorderedAccessView *m_pParticleUAV;
  ID3D11ShaderResourceView *m_pParticleSRV;

  // Constant buffer for simulation parameters
  ID3D11Buffer *m_pSimCB;

  // Gravity
  float m_vGravity[3];
};
