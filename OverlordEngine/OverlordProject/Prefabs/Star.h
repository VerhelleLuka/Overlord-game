#pragma once
#pragma once
#include <stdafx.cpp>
#include "Character.h"


class Star : public GameObject
{
public:
	Star() = default;
	~Star() override = default;

	Star(const Star& other) = delete;
	Star(Star&& other) noexcept = delete;
	Star& operator=(const Star& other) = delete;
	Star& operator=(Star&& other) noexcept = delete;

	void SetParticle(ParticleEmitterComponent* particle) { m_pParticle = particle; }

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	
		//Animation stuff
	ModelComponent* m_pModelComponent;
	float m_Rotation;

	//Particle reference
	ParticleEmitterComponent* m_pParticle;
};

