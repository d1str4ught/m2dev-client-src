#pragma once
#include "UserInterface/Locale_inc.h"
#include "Type.h"
#include "Eterlib/GrpBase.h"
#include "EterLib/Pool.h"

#include <array>

class CParticleProperty;
class CEmitterProperty;

class CParticleInstance
{
	friend class CParticleSystemData;
	friend class CParticleSystemInstance;

	public:
		CParticleInstance();
		~CParticleInstance();

		float GetRadiusApproximation();
		
		BOOL Update(float fElapsedTime, float fAngle);

	private:
		void UpdateRotation(float time, float elapsedTime);
		void UpdateTextureAnimation(float time, float elapsedTime);
		void UpdateScale(float time, float elapsedTime);
		void UpdateColor(float time, float elapsedTime);
		void UpdateGravity(float time, float elapsedTime);
		void UpdateAirResistance(float time, float elapsedTime);

	protected:
		D3DXVECTOR3			m_v3StartPosition;

		D3DXVECTOR3			m_v3Position;
		D3DXVECTOR3			m_v3LastPosition;
		D3DXVECTOR3			m_v3Velocity;

		D3DXVECTOR2			m_v2HalfSize;
		D3DXVECTOR2			m_v2Scale;

		float				m_fRotation;

		D3DXCOLOR			m_Color;

		BYTE				m_byTextureAnimationType;
		float				m_fLastFrameTime;
		BYTE				m_byFrameIndex;
		float				m_fFrameTime;

		float				m_fLifeTime;
		float				m_fLastLifeTime;

		CParticleProperty *	m_pParticleProperty;
		CEmitterProperty *	m_pEmitterProperty;

		BYTE				m_rotationType;

		float				m_fAirResistance;
		float				m_fRotationSpeed;
		float				m_fGravity;

	public:
		static CParticleInstance* New();
		static void DestroySystem();

		void Transform(const D3DXMATRIX * c_matLocal=NULL);
		void Transform(const D3DXMATRIX * c_matLocal, const float c_fZRotation);

#ifdef ENABLE_BATCHED_PARTICLE_RENDERING
		const std::array<TPDTVertex, 6>& GetParticleMeshPointer();
#else
		const std::array<TPTVertex, 4>& GetParticleMeshPointer();
#endif
		
		void DeleteThis();

		void Destroy();

	protected:
		void __Initialize();

#ifdef ENABLE_BATCHED_PARTICLE_RENDERING
		std::array<TPDTVertex, 6>	m_ParticleMesh;
#else
		std::array<TPTVertex, 4>	m_ParticleMesh;
#endif
	public:
		static CDynamicPool<CParticleInstance> ms_kPool;
		
};
