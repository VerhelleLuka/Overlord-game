#pragma once
#include <stdafx.cpp>
#include "Character.h"
enum class GameMode
{
	FIRSTPERSON,
	THIRDPERSON
};

enum class MovementState
{
	IDLE,
	WALKING,
	RUNNING,
	JUMPING,
	FALLING,
	BACKFLIP,
	FRONTFLIP, 
};

class Mario : public GameObject
{
public:
	Mario(const CharacterDesc& characterDesc/*, ModelComponent* modelComp */);
	~Mario() override;

	Mario(const Mario& other) = delete;
	Mario(Mario&& other) noexcept = delete;
	Mario& operator=(const Mario& other) = delete;
	Mario& operator=(Mario&& other) noexcept = delete;

	void SetGameMode(GameMode gameMode);

	void SetParticle(ParticleEmitterComponent* particle) { m_pParticle = particle; }
	bool GetPaused() const { return m_IsPaused; }
	void SetPaused(bool isPaused) { m_IsPaused = isPaused; }

	void AddForce(float x, float y, float z);
	void SetYForce(float force) { m_TotalVelocity.y = force; }
protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	//Camera
	CameraComponent* m_pCameraComponent{};
	float m_CameraDistance;
	const float m_OriginalCameraDistance{ -15.f };
	ControllerComponent* m_pControllerComponent{};

	CharacterDesc m_CharacterDesc;
	float m_TotalPitch{}, m_TotalYaw{};				//Total camera Pitch(X) and Yaw(Y) rotation
	float m_MoveAcceleration{},						//Acceleration required to reach maxMoveVelocity after 1 second (maxMoveVelocity / moveAccelerationTime)
		m_FallAcceleration{},						//Acceleration required to reach maxFallVelocity after 1 second (maxFallVelocity / fallAccelerationTime)
		m_MoveSpeed{};								//MoveSpeed > Horizontal Velocity = MoveDirection * MoveVelocity (= TotalVelocity.xz)

	XMFLOAT3 m_TotalVelocity{};						//TotalVelocity with X/Z for Horizontal Movement AND Y for Vertical Movement (fall/jump)
	XMFLOAT3 m_CurrentDirection{};					//Current/Last Direction based on Camera forward/right (Stored for deacceleration)

	GameMode m_GameMode = GameMode::FIRSTPERSON;
	const float m_MinPitch = -8.5f;					//Measured values
	const float m_MaxPitch = 75.f;
	ModelComponent* m_pModelComponent;
	//Rotation of the character mesh before camera rotation
		//Animation stuff
	ModelAnimator* m_pAnimator{};

	int m_AnimationClipId{ 0 };
	float m_AnimationSpeed{ 1.f };

	bool m_WalkAnimSet = false;
	bool m_RunAnimSet = false;
	char** m_ClipNames{};
	int m_ClipCount{};
	//===============
	//Matrices
	const PxVec3 m_Down = { GetTransform()->GetUp().x * -1,
			GetTransform()->GetUp().y * -1,
			GetTransform()->GetUp().z * -1 };

	//Movement stuff
	//float m_RunSpeed;
	float m_WalkSpeed;
	bool m_IsRunning; //Need this for triple jump
	bool m_JustJumped = false;
	bool m_JustLanded = false;
	bool m_ThirdJumpPrimed = false;
	float m_LandedTimeDouble{}; //How long ago mario landed from single jump
	const float m_MaxLandedTimeDouble{0.2f}; //Max amount of time mario can jump after landing to initiate double and/or triple jump

	//Same logic but for triple jump
	float m_LandedTimeTriple{}; //How long ago mario landed from single jump
	const float m_MaxLandedTimeTriple{ 0.3f }; //Max amount of time mario can jump after landing to initiate double and/or triple jump

	//Long jump
	bool m_LongJump = false;
	const float m_LongJumpSpeed = 2.5f;

	//State stuff
	MovementState m_MovementState;
	MovementState m_PreviousState;
	bool m_MovementStateChanged;

	void CheckStateChanged();

	bool m_IsGrounded;
	float m_BackFlipSpeed = 25.f;

	//Particle reference
	ParticleEmitterComponent* m_pParticle;

	//Other stuff
	const float m_yPosOffset = -1.35f;
	const float m_Scale = 0.01f;

	//For pause menu
	bool m_IsPaused;

	FMOD::Sound* m_pHoohooSound{};
	FMOD::Sound* m_pYahooSound{};
	FMOD::ChannelGroup* m_pSoundEffectGroup{};

	//backflip
	float m_OriginalMaxMoveSpeed;

	//running
	const float m_RunSpeedMultiplier = 1.5f;

	//raycast
	const float m_RayCastDistance = 0.07f;

		const float m_MaxJumpSpeed = 26.f;

};

