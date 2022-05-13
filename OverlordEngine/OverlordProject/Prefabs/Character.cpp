#include "stdafx.h"
#include "Character.h"

Character::Character(const CharacterDesc& characterDesc) :
	m_CharacterDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime)
{}

void Character::Initialize(const SceneContext& /*sceneContext*/)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDesc.controller));

	//Camera
	const auto pCamera = AddChild(new FixedCamera());
	m_pCameraComponent = pCamera->GetComponent<CameraComponent>();
	m_pCameraComponent->SetActive(true); //Uncomment to make this camera the active camera

	pCamera->GetTransform()->Translate(0.f, m_CharacterDesc.controller.height * .5f, 0.f);
}

void Character::Update(const SceneContext& sceneContext)
{
	if (m_pCameraComponent->IsActive())
	{
		//constexpr float epsilon{ 0.01f }; //Constant that can be used to compare if a float is near zero

		//***************
		//HANDLE INPUT

		//## Input Gathering (move)
		XMFLOAT2 move = {}; //Uncomment

		bool isMovingX = false;
		bool isMovingZ = false;

		//move.y should contain a 1 (Forward) or -1 (Backward) based on the active input (check corresponding actionId in m_CharacterDesc)
		//Optional: if move.y is near zero (abs(move.y) < epsilon), you could use the ThumbStickPosition.y for movement

		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveForward))
		{
			move.y = 1;
			isMovingZ = true;
		}
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveBackward))
		{
			move.y = -1;
			isMovingZ = true;
		}

		//move.x should contain a 1 (Right) or -1 (Left) based on the active input (check corresponding actionId in m_CharacterDesc)
		//Optional: if move.x is near zero (abs(move.x) < epsilon), you could use the Left ThumbStickPosition.x for movement
		if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft))
		{
			move.x = -1;
			isMovingX = true;
		}
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight))
		{
			move.x = 1;
			isMovingX = true;

		}
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

		GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
		//********
		//MOVEMENT

		//## Horizontal Velocity (Forward/Backward/Right/Left)
		//Calculate the current move acceleration for this frame (m_MoveAcceleration * ElapsedTime)
		auto moveAccell = m_MoveAcceleration * elapsedTime;

		//If the character is moving (= input is pressed)

		if (isMovingZ)
		{
			//Calculate & Store the current direction (m_CurrentDirection) >> based on the forward/right vectors and the pressed input
			m_CurrentDirection = { XMVectorGetX(forward),0 , XMVectorGetZ(forward) };
			//Increase the current MoveSpeed with the current Acceleration (m_MoveSpeed)
			//Make sure the current MoveSpeed stays below the maximum MoveSpeed (CharacterDesc::maxMoveSpeed)
			m_MoveSpeed += moveAccell * move.y;
			if (m_MoveSpeed * move.y > m_CharacterDesc.maxMoveSpeed)
			{
				m_MoveSpeed = m_CharacterDesc.maxMoveSpeed * move.y;
			}



		}
		if (isMovingX)
		{
			{

				//Calculate & Store the current direction (m_CurrentDirection) >> based on the forward/right vectors and the pressed input
				XMVECTOR vector = XMVector3Normalize({ m_CurrentDirection.x + XMVectorGetX(right), 0, m_CurrentDirection.z + XMVectorGetZ(right) });
				m_CurrentDirection = { XMVectorGetX(vector),0 , XMVectorGetZ(vector) };
				//Increase the current MoveSpeed with the current Acceleration (m_MoveSpeed)
				//Make sure the current MoveSpeed stays below the maximum MoveSpeed (CharacterDesc::maxMoveSpeed)
				m_MoveSpeed += moveAccell * move.x;

				if (m_MoveSpeed * move.x > m_CharacterDesc.maxMoveSpeed)
				{
					m_MoveSpeed = m_CharacterDesc.maxMoveSpeed * move.x;
				}
			}
		}
		//Else (character is not moving, or stopped moving)
		if (!isMovingX && !isMovingZ)
		{
			m_MoveSpeed -= moveAccell;
			if (m_MoveSpeed < 0)
			{
				m_MoveSpeed = 0;
			}
		}

		//Now we can calculate the Horizontal Velocity which should be stored in m_TotalVelocity.xz
		//Calculate the horizontal velocity (m_CurrentDirection * MoveSpeed)
		XMFLOAT2 horizontalVelocity = { m_MoveSpeed * m_CurrentDirection.x, m_MoveSpeed * m_CurrentDirection.z };

		//Set the x/z component of m_TotalVelocity (horizontal_velocity x/z)
		//It's important that you don't overwrite the y component of m_TotalVelocity (contains the vertical velocity)
		m_TotalVelocity.x = horizontalVelocity.x;
		m_TotalVelocity.z = horizontalVelocity.y;


		//## Vertical Movement (Jump/Fall)
		//If the Controller Component is NOT grounded (= freefall)
		auto collisionFlags = m_pControllerComponent->GetCollisionFlags();
		if (!collisionFlags.isSet(PxControllerCollisionFlag::eCOLLISION_DOWN))
		{
		
			m_TotalVelocity.y -= m_FallAcceleration * elapsedTime;
			if (m_TotalVelocity.y < -m_CharacterDesc.maxFallSpeed)
			{
				m_TotalVelocity.y = -m_CharacterDesc.maxFallSpeed;
			}
		}
		//Decrease the y component of m_TotalVelocity with a fraction (ElapsedTime) of the Fall Acceleration (m_FallAcceleration)
		//Make sure that the minimum speed stays above -CharacterDesc::maxFallSpeed (negative!)
		//Else If the jump action is triggered
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump) )
		{
			m_TotalVelocity.y = m_CharacterDesc.JumpSpeed;
		}
		else
		{
			m_TotalVelocity.y = 0;
		}
		//Set m_TotalVelocity.y equal to CharacterDesc::JumpSpeed
		//Else (=Character is grounded, no input pressed)
		//m_TotalVelocity.y is zero

	//************
	//DISPLACEMENT
		XMFLOAT3 displacement = { m_TotalVelocity.x * elapsedTime, m_TotalVelocity.y * elapsedTime, m_TotalVelocity.z * elapsedTime };
		m_pControllerComponent->Move(displacement);
		//The displacement required to move the Character Controller (ControllerComponent::Move) can be calculated using our TotalVelocity (m/s)
		//Calculate the displacement (m) for the current frame and move the ControllerComponent

		//The above is a simple implementation of Movement Dynamics, adjust the code to further improve the movement logic and behaviour.
		//Also, it can be usefull to use a seperate RayCast to check if the character is grounded (more responsive)
	}
}

void Character::DrawImGui()
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