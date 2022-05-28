
#pragma once
#include <stdafx.cpp>
#include "Character.h"


class Coin : public GameObject
{
public:
	Coin() = default;
	~Coin() override = default;

	Coin(const Coin& other) = delete;
	Coin(Coin&& other) noexcept = delete;
	Coin& operator=(const Coin& other) = delete;
	Coin& operator=(Coin&& other) noexcept = delete;

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

