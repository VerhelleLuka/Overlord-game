#pragma once
#include <stdafx.cpp>
#include "Character.h"


class KoopaTroopa : public GameObject
{
public:
	KoopaTroopa();
	~KoopaTroopa() override = default;

	KoopaTroopa(const KoopaTroopa& other) = delete;
	KoopaTroopa(KoopaTroopa&& other) noexcept = delete;
	KoopaTroopa& operator=(const KoopaTroopa& other) = delete;
	KoopaTroopa& operator=(KoopaTroopa&& other) noexcept = delete;

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

	ModelAnimator* pAnimator{};

	int m_AnimationClipId{ 0 };
	float m_AnimationSpeed{ 1.f };

	char** m_ClipNames{};
	int m_ClipCount{};

	//Movement stuff
	float m_RunSpeed;
	float m_WalkSpeed;
	bool m_IsRunning; 
};

