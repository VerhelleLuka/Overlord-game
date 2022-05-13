#include "stdafx.h"
#include "MyGameScene.h"

#include "Prefabs/Mario.h"
#include "Prefabs/KoopaTroopa.h"
#include "Materials/ColorMaterial.h"

void MyGameScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;
	m_SceneContext.settings.drawGrid = false;

	//Ground Plane
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);
	GameSceneExt::CreatePhysXGroundPlane(*this, pDefaultMaterial);

	//Character
	CharacterDesc characterDesc{ pDefaultMaterial };
	characterDesc.actionId_MoveForward = CharacterMoveForward;
	characterDesc.actionId_MoveBackward = CharacterMoveBackward;
	characterDesc.actionId_MoveLeft = CharacterMoveLeft;
	characterDesc.actionId_MoveRight = CharacterMoveRight;
	characterDesc.actionId_Jump = CharacterJump;
	characterDesc.actionId_Run = CharacterRun;



	m_pCharacter = AddChild(new Mario(characterDesc));
	m_pCharacter->SetGameMode(GameMode::THIRDPERSON);
	m_pCharacter->GetTransform()->Translate(0.f, 20.f, 0.f);
	//AddChild(m_pCharacter);

	//const auto pKoopaTroopa = AddChild(new KoopaTroopa);
	//pKoopaTroopa->GetTransform()->Translate(0.f, 25.f, 0.f);
	//pKoopaTroopa->GetTransform()->Scale(100.f);
	//AddChild(pKoopaTroopa);
	//Simple Level
	const auto pLevelObject = AddChild(new GameObject());
	const auto pLevelMesh = pLevelObject->AddComponent(new ModelComponent(L"Meshes/Bobomb_Battlefield.ovm"));
	pLevelMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	const auto pLevelActor = pLevelObject->AddComponent(new RigidBodyComponent(true));
	const auto pPxTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Bobomb_Battlefield.ovpt");
	pLevelActor->AddCollider(PxTriangleMeshGeometry(pPxTriangleMesh, PxMeshScale({ 1.f, 1.f, 1.f })), *pDefaultMaterial);
	pLevelObject->GetTransform()->Scale(1.f, 1.f, 1.f);

	//Input
	auto inputAction = InputAction(CharacterMoveLeft, InputState::down, 'Q');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveRight, InputState::down, 'D');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveForward, InputState::down, 'Z');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveBackward, InputState::down, 'S');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterJump, InputState::pressed, VK_SPACE, -1, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);
}

void MyGameScene::OnGUI()
{
	m_pCharacter->DrawImGui();
}
