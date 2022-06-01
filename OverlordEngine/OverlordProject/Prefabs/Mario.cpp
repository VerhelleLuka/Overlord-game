#include "stdafx.h"
#include "Mario.h"
#include <Materials/DiffuseMaterial_Skinned.h>
#include <Materials/ColorMaterial.h>
#include <Materials/DiffuseMaterial.h>
#include <Materials/Shadow/DiffuseMaterial_Shadow_Skinned.h>

Mario::Mario(const CharacterDesc& characterDesc) :
	m_CharacterDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_IsPaused(false)

{}

Mario::~Mario()
{
	for (int i{ 0 }; i < m_ClipCount; ++i)
	{
		delete[] m_ClipNames[i];
	}

	delete[] m_ClipNames;
}
void Mario::Initialize(const SceneContext& sceneContext)
{
	//Mesh
	const auto pMarioObject = new GameObject;
	auto pMarioMesh = pMarioObject->AddComponent(new ModelComponent(L"Meshes/Mario.ovm"));
	pMarioObject->GetTransform()->Translate(0, m_yPosOffset, 0);
	pMarioObject->GetTransform()->Scale(m_Scale);
	m_pModelComponent = pMarioMesh;
	AddChild(pMarioObject);

	//Controller
	m_CharacterDesc.controller.height = .5f;
	m_CharacterDesc.controller.radius = .30f;
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDesc.controller));
	//Character desc
	m_CharacterDesc.rotationSpeed /= 2;
	m_CharacterDesc.maxMoveSpeed /= 2;
	m_WalkSpeed = m_CharacterDesc.maxMoveSpeed;
	//m_RunSpeed = m_WalkSpeed * 2;
	//Camera
	const auto pCamera = AddChild(new FixedCamera());
	m_pCameraComponent = pCamera->GetComponent<CameraComponent>();
	m_pCameraComponent->SetActive(true); //Uncomment to make this camera the active camera

	InputAction action;
	action.actionID = m_CharacterDesc.actionId_Run;
	action.keyboardCode = VK_LSHIFT;
	action.playerIndex = GamepadIndex::playerOne;
	action.triggerState = InputState::down;
	sceneContext.pInput->AddInputAction(action);

	action.actionID = m_CharacterDesc.actionId_Duck;
	action.keyboardCode = VK_LCONTROL;
	action.playerIndex = GamepadIndex::playerOne;
	action.triggerState = InputState::down;
	sceneContext.pInput->AddInputAction(action);

	//Texture
	const auto pSkinnedMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pSkinnedMaterial->SetDiffuseTexture(L"Textures/Mario_UV.png");

	pMarioMesh->SetMaterial(pSkinnedMaterial);

	////Animation
	if (m_pModelComponent != nullptr)
	{
		m_pAnimator = m_pModelComponent->GetAnimator();
		m_pAnimator->SetAnimation(m_AnimationClipId);
		m_pAnimator->SetAnimationSpeed(m_AnimationSpeed);
		//Gather Clip Names
		m_ClipCount = m_pAnimator->GetClipCount();
		m_ClipNames = new char* [m_ClipCount];
		for (auto i{ 0 }; i < m_ClipCount; ++i)
		{
			m_pAnimator->SetAnimation(i);
			auto clipName = StringUtil::utf8_encode(m_pAnimator->GetClipName());
			m_ClipNames[i] = new char[clipName.size() + 1];
			//strcpy_s(m_ClipNames[i], sizeof(m_ClipNames[i]), clipName.c_str());
		}
		m_pAnimator->SetAnimation(2);
	}
	m_pAnimator->Play();


	//m_pControllerComponent->GetPxController().set
	//SoundManager::Get()->GetSystem()->playSound()
	m_PreviousState = MovementState::IDLE;
	m_MovementState = MovementState::IDLE;

	//m_pControllerComponent->SetCollisionGroup(CollisionGroup::Group1);
	m_pControllerComponent->SetCollisionIgnoreGroup(CollisionGroup::Group3);

	//Sound
	auto pSoundSystem = SoundManager::Get()->GetSystem();

	pSoundSystem->createStream("Resources/Sounds/yahoo.wav", FMOD_DEFAULT, nullptr, &m_pYahooSound);
	pSoundSystem->createStream("Resources/Sounds/hoohoo.wav", FMOD_DEFAULT, nullptr, &m_pHoohooSound);

	m_OriginalMaxMoveSpeed = m_CharacterDesc.maxMoveSpeed;

	m_TotalYaw = 45.f;
	m_TotalPitch = 10.f;
	m_CameraDistance = m_OriginalCameraDistance;
}

void Mario::Update(const SceneContext& sceneContext)
{
	//m_pRigidBody->GetTransform()->Translate(GetTransform()->GetPosition());
	if (m_pCameraComponent->IsActive() && !m_IsPaused)
	{
		if ((m_MovementState != MovementState::JUMPING && m_MovementState != MovementState::BACKFLIP && m_MovementState != MovementState::FRONTFLIP) ||
			m_IsGrounded)
			m_MovementState = MovementState::IDLE;

		XMFLOAT2 look{ 0.f, 0.f }; //Uncomment

		if (InputManager::IsMouseButton(InputState::down, VK_LBUTTON))
		{
			POINT lookNotCasted = sceneContext.pInput->GetMouseMovement();
			look.x = static_cast<float>(lookNotCasted.x);
			look.y = static_cast<float>(lookNotCasted.y);
		}
		bool ducked = false;
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Duck))
		{
			ducked = true;
		}

		auto transform = GetTransform();

		XMVECTOR forward = XMLoadFloat3(&transform->GetForward());
		XMVECTOR right = XMLoadFloat3(&transform->GetRight());
	
		const auto elapsedTime = sceneContext.pGameTime->GetElapsed();
		m_TotalYaw += look.x * elapsedTime * m_CharacterDesc.rotationSpeed;
		m_TotalPitch += look.y * elapsedTime * m_CharacterDesc.rotationSpeed;
		auto cameraForward = m_pCameraComponent->GetTransform()->GetForward();

		//If gamemode is in thirdperson, the pitch should be limited, the calculations in here are
		//from radians to pi
		if (m_GameMode == GameMode::THIRDPERSON)
		{

			if (m_TotalPitch < m_MinPitch)
			{
				m_TotalPitch = m_MinPitch;
			}
			else if (m_TotalPitch > m_MaxPitch)
			{
				m_TotalPitch = m_MaxPitch;

			}
			PxRaycastBuffer raycastBuffer;

			auto newCameraForward = m_pCameraComponent->GetTransform()->GetForward();
			newCameraForward.x *= m_CameraDistance;
			newCameraForward.y *= m_CameraDistance;
			newCameraForward.z *= m_CameraDistance;

			PxVec3 camOrigin = { m_pCameraComponent->GetTransform()->GetWorldPosition().x,
			m_pCameraComponent->GetTransform()->GetWorldPosition().y,
			m_pCameraComponent->GetTransform()->GetWorldPosition().z };
			PxVec3 camForwardVec =
			{
				m_pCameraComponent->GetTransform()->GetForward().x,
				m_pCameraComponent->GetTransform()->GetForward().y,
				m_pCameraComponent->GetTransform()->GetForward().z
			};
			m_pCameraComponent->GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
			bool regularCalculation = true;

			PxVec3 playerOrigin = { GetTransform()->GetWorldPosition().x,
				GetTransform()->GetWorldPosition().y,
				GetTransform()->GetWorldPosition().z };

			//if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(camOrigin, camForwardVec,
			//	(m_CameraDistance * -1) - 5.f, raycastBuffer))
			//{
			//	if (static_cast<BaseComponent*>(raycastBuffer.getAnyHit(static_cast<PxU32>(0)).actor->userData)->GetGameObject()->GetTag() == L"Level")
			//	{
			//		regularCalculation = false;
			//		m_pCameraComponent->GetTransform()->Translate(XMVECTOR{ camOrigin.x - raycastBuffer.block.position.x
			//		, camOrigin.y - raycastBuffer.block.position.y
			//		, camOrigin.z- raycastBuffer.block.position.z });
			//	}
			//}
			if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(playerOrigin, camForwardVec * -1,
				(m_CameraDistance * -1), raycastBuffer))
			{
				if (static_cast<BaseComponent*>(raycastBuffer.getAnyHit(static_cast<PxU32>(0)).actor->userData)->GetGameObject()->GetTag() == L"Level")
				{
					regularCalculation = false;
					m_pCameraComponent->GetTransform()->Translate(XMVECTOR{ /*camOrigin.x -*/ raycastBuffer.block.position.x
					, /*camOrigin.y -*/ raycastBuffer.block.position.y
					, /*camOrigin.z -*/ raycastBuffer.block.position.z });
				}
			}
			if(regularCalculation)
			
			{
				cameraForward.x *= m_OriginalCameraDistance;
				cameraForward.y *= m_OriginalCameraDistance;

				cameraForward.z *= m_OriginalCameraDistance;
				//m_pCameraComponent->GetTransform()->Translate(cameraForward);
				m_pCameraComponent->GetTransform()->Translate(cameraForward);
			}
			m_pCameraComponent->GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);



			//PxRaycastBuffer raycastBuffer;
			PxVec3 origin = { m_pCameraComponent->GetTransform()->GetPosition().x,
			m_pCameraComponent->GetTransform()->GetPosition().y,
			m_pCameraComponent->GetTransform()->GetPosition().z };
			PxVec3 direction = { m_pCameraComponent->GetTransform()->GetForward().x,
			m_pCameraComponent->GetTransform()->GetForward().y,
			m_pCameraComponent->GetTransform()->GetForward().z };

		}
		else
		{
			GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
		}

		bool isMoving = false;
		XMFLOAT3 moveInput{};
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveForward))
		{
			moveInput.z = 1;
			isMoving = true;
		}
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveBackward))
		{
			moveInput.z = -1;
			isMoving = true;

		}


		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight))
		{
			moveInput.x = 1;
			isMoving = true;
		}
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft))
		{
			moveInput.x = -1;
			isMoving = true;
		}
		auto moveAccell = m_MoveAcceleration * elapsedTime;
		if (isMoving)
		{
			m_pParticle->SetActive(true);
		}
		else
		{
			m_pParticle->SetActive(false);
		}

		m_CharacterDesc.maxMoveSpeed = m_OriginalMaxMoveSpeed * 0.25f;

		if (!ducked)
		{
			m_CharacterDesc.maxMoveSpeed = m_OriginalMaxMoveSpeed;

		}
		PxVec3 origin = { m_pControllerComponent->GetFootPosition().x,
			m_pControllerComponent->GetFootPosition().y,
			m_pControllerComponent->GetFootPosition().z };
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Run))
		{
			m_IsRunning = true;
			if (!ducked)
			{
				moveAccell *= m_RunSpeedMultiplier;
				moveAccell *= m_RunSpeedMultiplier;
				m_CharacterDesc.maxMoveSpeed = m_CharacterDesc.maxMoveSpeed * m_RunSpeedMultiplier;
				m_pParticle->SetActive(true);
				//Don't run if not touching ground
				PxRaycastBuffer raycastBuffer;

				if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, .1f, raycastBuffer))
				{

					m_MovementState = MovementState::RUNNING;
				}
				if (!m_RunAnimSet)
				{
					m_RunAnimSet = true;
					m_WalkAnimSet = false;
				}
			}

		}
		
		else
		{
			if (moveInput.x != 0 || moveInput.z != 0)
			{
				//Don't run if not touching ground
				PxRaycastBuffer raycastBuffer;

				if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, .1f, raycastBuffer))
				{
					m_MovementState = MovementState::WALKING;
				}
			}
			m_IsRunning = false;
			//m_CharacterDesc.maxMoveSpeed = m_WalkSpeed;
		}
		auto camForward = XMVector3Normalize(XMLoadFloat3(&m_pCameraComponent->GetTransform()->GetForward()));
		auto camRight = XMVector3Normalize(XMLoadFloat3(&m_pCameraComponent->GetTransform()->GetRight()));

		auto moveDirection = XMVector3Normalize(XMVectorSetY(camForward * moveInput.z + camRight * moveInput.x, 0.f));
		if (isMoving)
		{
			m_MoveSpeed += moveAccell;
			if (abs(m_MoveSpeed) > m_CharacterDesc.maxMoveSpeed && !m_LongJump)
			{
				m_MoveSpeed = m_CharacterDesc.maxMoveSpeed;
			}
		}
		//PLAYER ROTATION
		if (isMoving)
		{
			auto camRotForward = XMVector3Normalize({ XMVectorGetX(camForward),0, XMVectorGetZ(camForward) });
			auto camRotRight = XMVector3Normalize({ XMVectorGetX(camRight),0, XMVectorGetZ(camRight) });

			auto movementDir = XMVector2Normalize({ moveInput.x, moveInput.z });
			auto movementForward = XMVectorSet(moveInput.z * XMVectorGetX(camRotForward), 0.f, moveInput.z * XMVectorGetZ(camRotForward), 0.f);
			auto movementRight = XMVectorSet(moveInput.x * XMVectorGetX(camRotRight), 0.f, moveInput.x * XMVectorGetZ(camRotRight), 0.f);
			auto movementTotal = movementForward + movementRight;
			float rotAngle = -atan2(XMVectorGetZ(movementTotal), XMVectorGetX(movementTotal));
			//Convert to degrees
			rotAngle = XMConvertToDegrees(rotAngle);
			m_pModelComponent->GetTransform()->Rotate(0.f, rotAngle - 90.f, 0.f);
		}
		//Else (character is not moving, or stopped moving)
		if (!isMoving && !m_LongJump)
		{
			if (!ducked)
			{
				m_MoveSpeed -= moveAccell;
			}
			//Decellerate slower when ducked
			else
			{
				m_MoveSpeed -= moveAccell / 4;
			}

			if (m_MoveSpeed < 0)
			{
				m_MoveSpeed = 0;
			}
		}

		XMStoreFloat3(&m_CurrentDirection, moveDirection);
		XMFLOAT2 horizontalVelocity = { m_MoveSpeed * m_CurrentDirection.x, m_MoveSpeed * m_CurrentDirection.z };

		if (!m_LongJump)
		{
			m_TotalVelocity.x = horizontalVelocity.x;
			m_TotalVelocity.z = horizontalVelocity.y;
		}
		PxRaycastBuffer raycastBuffer;

		if (!SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, m_RayCastDistance, raycastBuffer))
		{

			m_pParticle->SetActive(false);

			m_IsGrounded = false;
			//If the character is going down, set him to fall
			if (m_TotalVelocity.y < 0)
			{

				if (m_MovementState != MovementState::FRONTFLIP && m_MovementState != MovementState::BACKFLIP)
					m_MovementState = MovementState::FALLING;

			}
			//Falling
			if (!m_LongJump)
			{
				m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
			}
			//Gravity works slower in a long jump :)
			else
			{
				m_TotalVelocity.y -= (m_FallAcceleration / 2) * elapsedTime;
			}
			//Don't go over max fall speed
			if (m_TotalVelocity.y < -m_CharacterDesc.maxFallSpeed)
			{
				m_TotalVelocity.y = -m_CharacterDesc.maxFallSpeed;
			}
			if (m_JustLanded)
			{
				//Double jump logic
				m_LandedTimeDouble += elapsedTime;
				if (m_LandedTimeDouble >= m_MaxLandedTimeDouble)
				{

					m_JustLanded = false;
					m_LandedTimeDouble = 0;
					m_JustJumped = false;
				}
				//Triple jump logic
				m_LandedTimeTriple += elapsedTime;
				if (m_LandedTimeTriple >= m_MaxLandedTimeTriple)
				{

					m_LandedTimeTriple = 0;
					m_ThirdJumpPrimed = false;
					m_JustLanded = false;
				}
			}
		}
		else if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, m_RayCastDistance, raycastBuffer) && sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
		{
			m_IsGrounded = false;
			m_MovementState = MovementState::JUMPING;

			m_pParticle->SetActive(true);
			m_pParticle->SpawnNrOfParticles(150, sceneContext, m_pModelComponent->GetTransform()->GetForward().x, 1.f, m_pModelComponent->GetTransform()->GetForward().z);
			m_pParticle->SetActive(false);
			//Non long jump logic
			if (!ducked)
			{
				//Double jump
				m_TotalVelocity.y = m_CharacterDesc.JumpSpeed;
				if (m_JustJumped && !m_ThirdJumpPrimed)
				{
					SoundManager::Get()->GetSystem()->playSound(m_pHoohooSound, m_pSoundEffectGroup, false, nullptr);
					m_TotalVelocity.y += m_CharacterDesc.JumpSpeed * 0.33f;
					m_ThirdJumpPrimed = true;
				}
				//Third jump
				else if (m_ThirdJumpPrimed)
				{
					SoundManager::Get()->GetSystem()->playSound(m_pYahooSound, m_pSoundEffectGroup, false, nullptr);

					m_MovementState = MovementState::FRONTFLIP;

					m_TotalVelocity.y += m_CharacterDesc.JumpSpeed * 0.75f;
				}
				if (!m_ThirdJumpPrimed)
					m_JustJumped = true;
			}
			//Long jump logic
			else if (ducked && m_IsRunning)
			{
				m_TotalVelocity.y = m_CharacterDesc.JumpSpeed / 2;
				m_MoveSpeed = 7.5f;
				m_TotalVelocity.x = -m_MoveSpeed * m_pModelComponent->GetTransform()->GetForward().x * m_LongJumpSpeed;
				m_TotalVelocity.z = -m_MoveSpeed * m_pModelComponent->GetTransform()->GetForward().z * m_LongJumpSpeed;
				m_MoveSpeed = m_CharacterDesc.maxMoveSpeed * 0.75f;
				m_LongJump = true;
			}
			//Backflip logic
			else if (ducked)
			{
				m_MovementState = MovementState::BACKFLIP;
				m_TotalVelocity.y = m_CharacterDesc.JumpSpeed * 1.5f;
			}
		}
		else
		{
			m_pControllerComponent->Move(XMFLOAT3{ 0.f, -m_RayCastDistance * 2, 0.f });

			m_pParticle->SetActive(false);
			m_IsGrounded = true;
			m_LongJump = false;
			if (m_JustJumped)
			{
				m_LandedTimeDouble += elapsedTime;
				m_JustLanded = true;

			}
			if (m_LandedTimeDouble >= m_MaxLandedTimeDouble)
			{
				m_LandedTimeDouble = 0;
				m_JustJumped = false;
			}
			if (m_ThirdJumpPrimed)
			{
				m_JustLanded = true;
				m_LandedTimeTriple += elapsedTime;
				if (m_LandedTimeTriple >= m_MaxLandedTimeTriple)
				{
					m_LandedTimeTriple = 0;
					m_ThirdJumpPrimed = false;
					m_JustLanded = false;
				}
			}
			m_TotalVelocity.y = 0;
		}
		if (m_TotalVelocity.y > m_MaxJumpSpeed)
		{
			m_TotalVelocity.y = m_MaxJumpSpeed;
		}
		XMFLOAT3 displacement = { m_TotalVelocity.x * elapsedTime, m_TotalVelocity.y * elapsedTime, m_TotalVelocity.z * elapsedTime };
		m_pControllerComponent->Move(displacement);
		CheckStateChanged();
	}
}

void Mario::CheckStateChanged()
{
	if (m_PreviousState != m_MovementState)
	{
		switch (m_MovementState)
		{
		case MovementState::IDLE:
			m_pAnimator->SetAnimation(0);
			m_PreviousState = m_MovementState;
			break;
		case MovementState::WALKING:
			m_pAnimator->SetAnimation(6);
			m_PreviousState = m_MovementState;
			break;
		case MovementState::RUNNING:
			m_pAnimator->SetAnimation(5);
			m_PreviousState = m_MovementState;
			break;
		case MovementState::FALLING:
			m_pAnimator->SetAnimation(4);
			m_PreviousState = m_MovementState;
			break;
		case MovementState::JUMPING:
			m_pAnimator->SetAnimation(4);

			m_PreviousState = m_MovementState;
			break;
		case MovementState::FRONTFLIP:
			m_pAnimator->SetAnimation(3);
			m_PreviousState = m_MovementState;
			break;
		case MovementState::BACKFLIP:
			m_pAnimator->SetAnimation(1);
			m_PreviousState = m_MovementState;
			break;
		}
	}
}

void Mario::AddForce(float x, float y, float z)
{
	m_TotalVelocity.x += x;
	m_TotalVelocity.y += y;
	m_TotalVelocity.z += z;
}



void Mario::SetGameMode(GameMode gameMode)
{
	m_GameMode = gameMode;
	switch (gameMode)
	{
	case GameMode::THIRDPERSON:
		m_pCameraComponent->GetTransform()->Translate(m_pCameraComponent->GetTransform()->GetForward().x,
			m_pCameraComponent->GetTransform()->GetForward().y ,
			m_pCameraComponent->GetTransform()->GetForward().z - m_CameraDistance);
		break;
	}
}