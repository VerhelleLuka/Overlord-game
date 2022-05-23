#include "stdafx.h"
#include "MyGameScene.h"

#include "Prefabs/Mario.h"
#include "Prefabs/KoopaTroopa.h"
#include "Materials/ColorMaterial.h"
#include <Materials/PostPixelation.h>
#include <Materials/PostGrayscale.h>
#include "Components/SpriteComponent.h"
void MyGameScene::Initialize()
{
	//m_SceneContext.settings.enableOnGUI = true;
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

	//Post processing effect
	m_SceneInitialized = false;
	m_NrPixels = 2;
	m_pPixelation = MaterialManager::Get()->CreateMaterial<PostPixelation>();
	AddPostProcessingEffect(m_pPixelation);
	m_pPixelation->SetIsEnabled(true);
	m_pPixelation->SetNrPixels(m_NrPixels);

	//m_pGrayscale = MaterialManager::Get()->CreateMaterial<PostGrayscale>();
	//AddPostProcessingEffect(m_pGrayscale);
	//m_pGrayscale->SetIsEnabled(true);
	m_pPixelation->IncreasePixelation();

	//Sprite
	for (int i{}; i < 6; ++i)
	{
		m_pUI[i] = new GameObject();
		m_pUI[i]->AddComponent(new SpriteComponent(L"Textures/TestSprite.png", { 0.5f, 0.5f }, { 1.f,1.f ,1.f ,1.f }));
		AddChild(m_pUI[i]);
		m_pUI[i]->GetComponent<SpriteComponent>()->GetTransform()->Translate((m_SceneContext.windowWidth / 50.f) * ((i + 1) * 1.1f), m_SceneContext.windowHeight / 10.f, .9f);
		m_pUI[i]->GetComponent<SpriteComponent>()->GetTransform()->Rotate(0, 0, (360.f / ((i % 2) + 1)), true);
	}

	//Particle 
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,6.f,0.f };
	settings.minSize = .5f;
	settings.maxSize = .1f;
	settings.minEnergy = 1.f;
	settings.maxEnergy = 2.f;
	settings.minScale = 1.f;
	settings.maxScale = 2.f;
	settings.minEmitterRadius = .2f;
	settings.maxEmitterRadius = .5f;
	settings.color = { 1.f,1.f,1.f, .6f };

	m_pEmitter = m_pCharacter->AddComponent(new ParticleEmitterComponent(L"Textures/Smoke.png", settings, 100));
	m_pCharacter->SetParticle(m_pEmitter);
	
}

void MyGameScene::Update()
{
	if (!m_SceneInitialized)
	{
		m_PixelationTimer += m_SceneContext.pGameTime->GetElapsed();
		if (m_pPixelation->GetNrPixels() < 500)
		{
			if (m_PixelationTimer >= m_PixelationTime)
			{
				m_PixelationTimer = 0;
				m_pPixelation->IncreasePixelation();
			}
		}
		else
		{
			m_SceneInitialized = true;
			m_pPixelation->SetIsEnabled(false);
		}
	}
}

void MyGameScene::OnGUI()
{
	//m_pCharacter->DrawImGui();
}
