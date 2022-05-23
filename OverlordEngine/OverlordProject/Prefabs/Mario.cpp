#include "stdafx.h"
#include "Mario.h"
#include <Materials/DiffuseMaterial_Skinned.h>
#include <Materials/ColorMaterial.h>
#include <Materials/DiffuseMaterial.h>

Mario::Mario(const CharacterDesc& characterDesc) :
	m_CharacterDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime)

{}

void Mario::Initialize(const SceneContext& sceneContext)
{
	//Mesh
	const auto pMarioObject = new GameObject;
	auto pMarioMesh = pMarioObject->AddComponent(new ModelComponent(L"Meshes/Mario/Mario.ovm"));
	pMarioMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	pMarioMesh->GetTransform()->Rotate(0.f, 180.f, 0.f);
	pMarioObject->GetTransform()->Scale(0.005f);
	m_pModelComponent = pMarioMesh;
	AddChild(pMarioObject);

	//Controller
	m_CharacterDesc.controller.height = .6f;
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDesc.controller));
	//Character desc
	m_CharacterDesc.rotationSpeed /= 2;
	m_CharacterDesc.maxMoveSpeed /= 2;
	m_WalkSpeed = m_CharacterDesc.maxMoveSpeed;
	m_RunSpeed = m_WalkSpeed * 2;
	//Camera
	const auto pCamera = AddChild(new FixedCamera());
	m_pCameraComponent = pCamera->GetComponent<CameraComponent>();
	m_pCameraComponent->SetActive(true); //Uncomment to make this camera the active camera

	//Add mario controls
	//pCamera->GetTransform()->Translate(0.f, m_CharacterDesc.controller.height * .5f, 0.f);
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
	const auto pSkinnedMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	pSkinnedMaterial->SetDiffuseTexture(L"Textures/Mario_UV.png");

	m_pModelComponent->SetMaterial(pSkinnedMaterial);

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

	//Set foot position 1.6 higher due to offset
	PxVec3 origin = { m_pControllerComponent->GetFootPosition().x,
m_pControllerComponent->GetFootPosition().y,
m_pControllerComponent->GetFootPosition().z };
	origin.y += 1.6f;
	PxExtendedVec3 origin2;
	origin2.x = origin.x;
	origin2.y = origin.y;
	origin2.z = origin.z;
	m_pControllerComponent->GetPxController()->setFootPosition(origin2);

	//m_pControllerComponent->GetPxController().set
	//SoundManager::Get()->GetSystem()->playSound()
	m_PreviousState = MovementState::IDLE;
	m_MovementState = MovementState::IDLE;
}

void Mario::Update(const SceneContext& sceneContext)
{
	if (m_pCameraComponent->IsActive())
	{
		if ((m_MovementState != MovementState::JUMPING && m_MovementState != MovementState::BACKFLIP && m_MovementState != MovementState::FRONTFLIP) ||
			m_IsGrounded)
			m_MovementState = MovementState::IDLE;
		//constexpr float epsilon{ 0.01f }; //Constant that can be used to compare if a float is near zero

		//***************
		//HANDLE INPUT

		//## Input Gathering (look)
		XMFLOAT2 look{ 0.f, 0.f }; //Uncomment
		//Only if the Left Mouse Button is Down >
			// Store the MouseMovement in the local 'look' variable (cast is required)
		//Optional: in case look.x AND look.y are near zero, you could use the Right ThumbStickPosition for look
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
		//************************
		//GATHERING TRANSFORM INFO

		//Retrieve the TransformComponent
		//Retrieve the forward & right vector (as XMVECTOR) from the TransformComponent
		auto transform = GetTransform();

		XMVECTOR forward = XMLoadFloat3(&transform->GetForward());
		XMVECTOR right = XMLoadFloat3(&transform->GetRight());
		//***************
		//CAMERA ROTATION
		//Adjust the TotalYaw (m_TotalYaw) & TotalPitch (m_TotalPitch) based on the local 'look' variable
		//Make sure this calculated on a framerate independent way and uses CharacterDesc::rotationSpeed.
		//Rotate this character based on the TotalPitch (X) and TotalYaw (Y)
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
			// *-1 to get inverse forward vecter *20 to get distance
			cameraForward.x *= -10.f;
			cameraForward.y *= -10.f;
			cameraForward.z *= -10.f;
			//float distance = 20.f;
			m_pCameraComponent->GetTransform()->Translate(GetTransform()->GetPosition().x, GetTransform()->GetPosition().y, GetTransform()->GetPosition().z);
			m_pCameraComponent->GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
			//cameraForward.x += GetTransform()->GetPosition().x;
			//cameraForward.y += GetTransform()->GetPosition().y;
			//cameraForward.z += GetTransform()->GetPosition().z;
			m_pCameraComponent->GetTransform()->Translate(cameraForward);
		}
		else
		{
			GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
		}
		//********
		//MOVEMENT
		//## Horizontal Velocity (Forward/Backward/Right/Left)
		//Calculate the current move acceleration for this frame (m_MoveAcceleration * ElapsedTime)

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
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Run))
		{
			m_IsRunning = true;
			moveAccell *= 2;
			moveAccell *= 2;
			m_CharacterDesc.maxMoveSpeed = m_RunSpeed;

			//Don't run if not touching ground
			PxRaycastBuffer raycastBuffer;
			PxVec3 origin = { m_pControllerComponent->GetFootPosition().x,
			m_pControllerComponent->GetFootPosition().y,
			m_pControllerComponent->GetFootPosition().z };
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
		else
		{
			if (moveInput.x != 0 || moveInput.z != 0)
			{
				//Don't run if not touching ground
				PxRaycastBuffer raycastBuffer;
				PxVec3 origin = { m_pControllerComponent->GetFootPosition().x,
				m_pControllerComponent->GetFootPosition().y,
				m_pControllerComponent->GetFootPosition().z };
				if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, .1f, raycastBuffer))
				{
					m_MovementState = MovementState::WALKING;
				}
			}
			m_IsRunning = false;
			m_CharacterDesc.maxMoveSpeed = m_WalkSpeed;
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
			rotAngle = (rotAngle * 180.f) / 3.1415f;
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
		//Now we can calculate the Horizontal Velocity which should be stored in m_TotalVelocity.xz
		//Calculate the horizontal velocity (m_CurrentDirection * MoveSpeed)
		XMStoreFloat3(&m_CurrentDirection, moveDirection);
		XMFLOAT2 horizontalVelocity = { m_MoveSpeed * m_CurrentDirection.x, m_MoveSpeed * m_CurrentDirection.z };

		//Set the x/z component of m_TotalVelocity (horizontal_velocity x/z)
		//It's important that you don't overwrite the y component of m_TotalVelocity (contains the vertical velocity)
		m_TotalVelocity.x = horizontalVelocity.x;
		m_TotalVelocity.z = horizontalVelocity.y;


		//## Vertical Movement (Jump/Fall)
		//If the Controller Component is NOT grounded (= freefall)
		auto collisionFlags = m_pControllerComponent->GetCollisionFlags();

		PxRaycastBuffer raycastBuffer;
		PxVec3 origin = { m_pControllerComponent->GetFootPosition().x,
		m_pControllerComponent->GetFootPosition().y,
		m_pControllerComponent->GetFootPosition().z };

		//Direction is the down vector (inverse of up vector)


		//Decrease the y component of m_TotalVelocity with a fraction (ElapsedTime) of the Fall Acceleration (m_FallAcceleration)
		//Make sure that the minimum speed stays above -CharacterDesc::maxFallSpeed (negative!)
		//Else If the jump action is triggered

		//This part is very fragile with all the booleans, please don't mess with it --
		if (!SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, .1f, raycastBuffer))
		{
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
		else if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, m_Down, .1f, raycastBuffer) && sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump))
		{
			m_IsGrounded = false;
			m_MovementState = MovementState::JUMPING;

			//Non long jump logic
			if (!ducked)
			{
				//Double jump
				m_TotalVelocity.y = m_CharacterDesc.JumpSpeed;
				if (m_JustJumped && !m_ThirdJumpPrimed)
				{
					m_TotalVelocity.y += m_CharacterDesc.JumpSpeed;
					m_ThirdJumpPrimed = true;
				}
				//Third jump
				else if (m_ThirdJumpPrimed)
				{
					m_MovementState = MovementState::FRONTFLIP;

					m_TotalVelocity.y += m_CharacterDesc.JumpSpeed * 2;
				}
				if (!m_ThirdJumpPrimed)
					m_JustJumped = true;
			}
			//Long jump logic
			else if (ducked && m_IsRunning)
			{
				m_TotalVelocity.y = m_CharacterDesc.JumpSpeed / 2;
				m_TotalVelocity.x = -m_MoveSpeed * m_pModelComponent->GetTransform()->GetForward().x * m_LongJumpSpeed;
				m_TotalVelocity.z = -m_MoveSpeed * m_pModelComponent->GetTransform()->GetForward().z * m_LongJumpSpeed;
				m_MoveSpeed = m_CharacterDesc.maxMoveSpeed * 0.75f;
				m_CurrentDirection = { m_pModelComponent->GetTransform()->GetForward().x, m_pModelComponent->GetTransform()->GetForward().y,m_pModelComponent->GetTransform()->GetForward().z };
				m_LongJump = true;
			}
			//Backflip logic
			else if (ducked)
			{
				m_MovementState = MovementState::BACKFLIP;

				m_TotalVelocity.y = m_CharacterDesc.JumpSpeed * 1.5f;
				m_TotalVelocity.x = m_MoveSpeed * -m_CurrentDirection.x * m_BackFlipSpeed;
				m_TotalVelocity.z = m_MoveSpeed * -m_CurrentDirection.z * m_BackFlipSpeed;
			}
		}
		else
		{
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
		//Do another raycast to check if the character should be falling because .25f is too shallow

		//Finnicky till here
		// 
		//************
		//DISPLACEMENT
		XMFLOAT3 displacement = { m_TotalVelocity.x * elapsedTime, m_TotalVelocity.y * elapsedTime, m_TotalVelocity.z * elapsedTime };
		m_pControllerComponent->Move(displacement);
		//The displacement required to move the Character Controller (ControllerComponent::Move) can be calculated using our TotalVelocity (m/s)
		//Calculate the displacement (m) for the current frame and move the ControllerComponent

		//The above is a simple implementation of Movement Dynamics, adjust the code to further improve the movement logic and behaviour.
		//Also, it can be usefull to use a seperate RayCast to check if the character is grounded (more responsive)
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

void Mario::DrawImGui()
{
	if (ImGui::CollapsingHeader("Character"))
	{
		ImGui::Text(std::format("Move Speed: {:0.1f} m/s", m_MoveSpeed).c_str());
		ImGui::Text(std::format("Fall Speed: {:0.1f} m/s", m_TotalVelocity.y).c_str());

		ImGui::Text(std::format("Move Acceleration: {:0.1f} m/s2", m_MoveAcceleration).c_str());
		ImGui::Text(std::format("Fall Acceleration: {:0.1f} m/s2", m_FallAcceleration).c_str());

		const float jumpMaxTime = m_CharacterDesc.JumpSpeed / m_FallAcceleration;
		const float jumpMaxHeight = (m_CharacterDesc.JumpSpeed * jumpMaxTime) - (0.5f * (m_FallAcceleration * powf(jumpMaxTime, 2)));
		ImGui::Text(std::format("Jump Height: {:0.1f} m", jumpMaxHeight).c_str());

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Move Speed (m/s)", &m_CharacterDesc.maxMoveSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Move Acceleration Time (s)", &m_CharacterDesc.moveAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_MoveAcceleration = m_CharacterDesc.maxMoveSpeed / m_CharacterDesc.moveAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Fall Speed (m/s)", &m_CharacterDesc.maxFallSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Fall Acceleration Time (s)", &m_CharacterDesc.fallAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_FallAcceleration = m_CharacterDesc.maxFallSpeed / m_CharacterDesc.fallAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		ImGui::DragFloat("Jump Speed", &m_CharacterDesc.JumpSpeed, 0.1f, 0.f, 0.f, "%.1f");
		ImGui::DragFloat("Rotation Speed (deg/s)", &m_CharacterDesc.rotationSpeed, 0.1f, 0.f, 0.f, "%.1f");

		bool isActive = m_pCameraComponent->IsActive();
		if (ImGui::Checkbox("Character Camera", &isActive))
		{
			m_pCameraComponent->SetActive(isActive);
		}
	}
}

void Mario::SetGameMode(GameMode gameMode)
{
	m_GameMode = gameMode;
	switch (gameMode)
	{
	case GameMode::THIRDPERSON:
		m_pCameraComponent->GetTransform()->Translate(m_pCameraComponent->GetTransform()->GetForward().x,
			m_pCameraComponent->GetTransform()->GetForward().y + 1.f,
			m_pCameraComponent->GetTransform()->GetForward().z - 10.f);
		break;
	}
}