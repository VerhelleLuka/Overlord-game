#include "stdafx.h"
#include "MyGameScene.h"
#include "Prefabs/Mario.h"
#include "Prefabs/KoopaTroopa.h"
#include "Materials/ColorMaterial.h"
#include <Materials/PostPixelation.h>
#include <Materials/PostGrayscale.h>
#include "Components/SpriteComponent.h"
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>
#include "Prefabs/Star.h"
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
	m_pCharacter->GetTransform()->Translate(m_OriginalPosition);
	m_pCharacter->GetTransform()->Scale(0.5f, 0.5f, 0.5f);
	//AddChild(m_pCharacter);

	//const auto pKoopaTroopa = AddChild(new KoopaTroopa);
	//pKoopaTroopa->GetTransform()->Translate(0.f, 25.f, 0.f);
	//pKoopaTroopa->GetTransform()->Scale(100.f);
	//AddChild(pKoopaTroopa);

	//Simple Level
	CreateLevel();

	//END OF LEVEL CREATION
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
	settings.velocity = { 1.f,1.f,1.f };
	settings.minSize = .1f;
	settings.maxSize = .5f;
	settings.minEnergy = .5f;
	settings.maxEnergy = 1.f;
	settings.minScale = .5f;
	settings.maxScale = 1.f;
	settings.minEmitterRadius = .2f;
	settings.maxEmitterRadius = .5f;
	settings.color = { 1.f,1.f,1.f, .6f };

	m_pEmitter = m_pCharacter->AddComponent(new ParticleEmitterComponent(L"Textures/Jump.png", settings, 10));
	m_pCharacter->SetParticle(m_pEmitter);

	//Light
	m_SceneContext.pLights->SetDirectionalLight({ 0,66.1346436f,-21.1850471f }, { -0.740129888f, 0.597205281f, 0.309117377f });

	//Pause menu
	auto pPauseGo = new GameObject();
	m_pPauseMenu = new SpriteComponent(L"Textures/PauseMenu.png");
	pPauseGo->AddComponent(m_pPauseMenu);
	m_pPauseMenu->Enable(false);
	AddChild(pPauseGo);

	//Star
	auto pStarGo = new Star();
	AddChild(pStarGo);
	auto starBody = pStarGo->AddComponent(new RigidBodyComponent());
	starBody->AddCollider(PxBoxGeometry{ 1.f, 1.f, 1.f }, *pDefaultMaterial, true);
	pStarGo->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//starBody->SetKinematic(true);	
	pStarGo->GetTransform()->Translate(m_OriginalPosition);
}

void MyGameScene::CreateLevel()
{
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);
	
		//LEVEL MATERIALS
		const auto pDirtMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pDirtMat->SetDiffuseTexture(L"Textures/Dirt.bmp");

		const auto pGrassMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pGrassMat->SetDiffuseTexture(L"Textures/Grass.bmp");

		const auto pWalkMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pWalkMat->SetDiffuseTexture(L"Textures/Pathway.bmp");

		const auto pWoodMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pWoodMat->SetDiffuseTexture(L"Textures/Wood.bmp");

		const auto pFancyCobble = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pFancyCobble->SetDiffuseTexture(L"Textures/FancyCobble.bmp");

		const auto pCobble = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pCobble->SetDiffuseTexture(L"Textures/Cobble.bmp");

		const auto pRock = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pRock->SetDiffuseTexture(L"Textures/Rock.bmp");

		const auto pCaveRock = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pCaveRock->SetDiffuseTexture(L"Textures/CaveRock.bmp");

		const auto pMarble = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pMarble->SetDiffuseTexture(L"Textures/Marble.bmp");

		const auto pFlowerStems = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pFlowerStems->SetDiffuseTexture(L"Textures/FlowerStems.bmp");

		const auto pFlowers = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pFlowers->SetDiffuseTexture(L"Textures/Flower.bmp");

		const auto pThingy = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pThingy->SetDiffuseTexture(L"Textures/Thingy.bmp");

		const auto pPlatform = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pPlatform->SetDiffuseTexture(L"Textures/Platform.bmp");

		const auto pCavernRock = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pCavernRock->SetDiffuseTexture(L"Textures/CavernRock.bmp");

		const auto pMountainRock = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pMountainRock->SetDiffuseTexture(L"Textures/MountainRock.bmp");

		const auto pBars = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pBars->SetDiffuseTexture(L"Textures/Bars.png");

		const auto pBarbedWire = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
		pBarbedWire->SetDiffuseTexture(L"Textures/BarbedWire.png");
	//const auto pGrassMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	//pGrassMat->SetDiffuseTexture(L"Textures/Grass.bmp");
	//LEVEL
	const auto pLevelObject = AddChild(new GameObject());
	const auto pLevelMesh = pLevelObject->AddComponent(new ModelComponent(L"Meshes/Bobomb_Battlefield2.ovm"));
	//SET MATERIALS

	pLevelMesh->SetMaterial(pGrassMat, 22);
	pLevelMesh->SetMaterial(pWalkMat, 21);
	pLevelMesh->SetMaterial(pWoodMat, 20);
	pLevelMesh->SetMaterial(pFancyCobble, 19);
	pLevelMesh->SetMaterial(pPlatform, 18);
	pLevelMesh->SetMaterial(pGrassMat, 17);
	pLevelMesh->SetMaterial(pRock, 16);
	pLevelMesh->SetMaterial(pRock, 15);
	pLevelMesh->SetMaterial(pCaveRock, 14);
	pLevelMesh->SetMaterial(pMarble, 13);
	pLevelMesh->SetMaterial(pFlowers, 11);
	pLevelMesh->SetMaterial(pCobble, 12);
	pLevelMesh->SetMaterial(pFlowerStems, 10);
	pLevelMesh->SetMaterial(pMountainRock, 9);
	pLevelMesh->SetMaterial(pMountainRock, 8);
	pLevelMesh->SetMaterial(pGrassMat, 7);
	pLevelMesh->SetMaterial(pWalkMat, 6);
	pLevelMesh->SetMaterial(pThingy, 5);
	pLevelMesh->SetMaterial(pDirtMat, 4);
	pLevelMesh->SetMaterial(pWoodMat, 3);
	pLevelMesh->SetMaterial(pCavernRock, 2);
	pLevelMesh->SetMaterial(pBars, 1);
	pLevelMesh->SetMaterial(pBarbedWire, 0);



	const auto pLevelActor = pLevelObject->AddComponent(new RigidBodyComponent(true));
	pLevelObject->GetComponent<RigidBodyComponent>()->GetTransform()->Scale(0.5f, 0.5f, 0.5f);
	const auto pPxTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Bobomb_Battlefield.ovpt");
	pLevelActor->AddCollider(PxTriangleMeshGeometry(pPxTriangleMesh, PxMeshScale({ 1.f, 1.f, 1.f })), *pDefaultMaterial);

	pLevelObject->GetTransform()->Scale(1.f, 1.f, 1.f);
}
void MyGameScene::Update()
{
	m_pPauseMenu->GetTexture()->SetDimenson({ m_SceneContext.windowWidth, m_SceneContext.windowHeight });
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
	if (InputManager::IsKeyboardKey(InputState::pressed, VK_ESCAPE) )
	{
		if (!m_pCharacter->GetPaused())
		{
			m_pCharacter->SetPaused(true);
			m_pPauseMenu->Enable(true);
		}
		else
		{
			m_pCharacter->SetPaused(false);
			m_pPauseMenu->Enable(false);
		}

	}
	if (InputManager::IsKeyboardKey(InputState::pressed, VK_SPACE) && m_pCharacter->GetPaused())
	{
		m_pCharacter->GetTransform()->Translate(m_OriginalPosition);
		m_pPauseMenu->Enable(false);
		m_pCharacter->SetPaused(false);


	}
	if (InputManager::IsKeyboardKey(InputState::pressed, VK_RETURN) && m_pCharacter->GetPaused())
	{
		m_pCharacter->GetTransform()->Translate(m_OriginalPosition);
		m_pCharacter->SetPaused(false);
		m_pPauseMenu->Enable(false);
		SceneManager::Get()->PreviousScene();
	}
	if (m_pCharacter->GetComponent<RigidBodyComponent>(true))
	{
		std::cout << "Has rigid body\n";
	}
}

void MyGameScene::OnGUI()
{
	//ImGui::SliderInt("foij", &m_MaterialID, 0, 22);
	//m_pLevelMode->SetMaterial(m_pMat, UINT8(m_MaterialID));

}

void MyGameScene::OnTriggerCallBack(GameObject* /*pTriggerObject*/, GameObject* /*pOtherObject*/, PxTriggerAction /*action*/)
{
	std::cout << "Star colliding!\n";
}
