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

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	
	float m_MoveAcceleration{},						//Acceleration required to reach maxMoveVelocity after 1 second (maxMoveVelocity / moveAccelerationTime)
		m_FallAcceleration{},						//Acceleration required to reach maxFallVelocity after 1 second (maxFallVelocity / fallAccelerationTime)
		m_MoveSpeed{};								//MoveSpeed > Horizontal Velocity = MoveDirection * MoveVelocity (= TotalVelocity.xz)

	XMFLOAT3 m_TotalVelocity{};						//TotalVelocity with X/Z for Horizontal Movement AND Y for Vertical Movement (fall/jump)
	XMFLOAT3 m_CurrentDirection{};					//Current/Last Direction based on Camera forward/right (Stored for deacceleration)

		//Animation stuff
	ModelComponent* m_pModelComponent;
};

