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
#include "Prefabs/Goomba.h"
#include "Prefabs/Coin.h"

void MyGameScene::Initialize()
{
	m_KoopasKilled = 0;
	m_SceneContext.settings.enableOnGUI = true;
	m_SceneContext.settings.drawGrid = false;
	//m_SceneContext.settings.showInfoOverlay = false;

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
	m_pCharacter->SetTag(L"Mario");
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
	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, 0.597205281f, 0.309117377f });

	//Pause menu
	auto pPauseGo = new GameObject();
	m_pPauseMenu = new SpriteComponent(L"Textures/PauseMenu.png");
	pPauseGo->AddComponent(m_pPauseMenu);
	m_pPauseMenu->Enable(false);
	AddChild(pPauseGo);
	//Death screen
	auto pDeathGo = new GameObject();
	m_pDeathScreen = new SpriteComponent(L"Textures/Death.png");
	pDeathGo->AddComponent(m_pDeathScreen);
	m_pDeathScreen->Enable(false);
	AddChild(pDeathGo);
	//Win screen
	auto pWinGo = new GameObject();
	m_pWinScreen = new SpriteComponent(L"Textures/winscreen.png");
	pWinGo->AddComponent(m_pWinScreen);
	m_pWinScreen->Enable(false);
	AddChild(pWinGo);

	//Sound effects
	auto pSoundSystem = SoundManager::Get()->GetSystem();

	pSoundSystem->createStream("Resources/Sounds/gameover.wav", FMOD_DEFAULT, nullptr, &m_pGameOverSound);
	pSoundSystem->createStream("Resources/Sounds/oof.wav", FMOD_DEFAULT, nullptr, &m_pOofSound);
	pSoundSystem->createStream("Resources/Sounds/star_appears.wav", FMOD_DEFAULT, nullptr, &m_pStarSound);
	pSoundSystem->createStream("Resources/Sounds/pause.wav", FMOD_DEFAULT, nullptr, &m_pPauseSound);
	pSoundSystem->createStream("Resources/Sounds/Coin.wav", FMOD_DEFAULT, nullptr, &m_pCoinSound);

	CreateStar();
	CreateCoins();
	CreateKoopaTroopas();

	//Font
	m_pFont = ContentManager::Load<SpriteFont>(L"SpriteFonts/Consolas_32.fnt");
	m_TextPos.x = 150.f;
	m_TextPos.y = 300.f;

	m_NrCoins = 0;

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
	pLevelObject->SetTag(L"Level");
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

void MyGameScene::CreateStar()
{
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	auto pStarGo = new Star();
	pStarGo->GetTransform()->Translate(0.f, -10.f, 0.f);

	AddChild(pStarGo);
	auto starBody = pStarGo->AddComponent(new RigidBodyComponent());
	starBody->AddCollider(PxBoxGeometry{ 1.f, 1.f, 1.f }, *pDefaultMaterial, true);
	pStarGo->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	starBody->SetCollisionGroup(CollisionGroup::Group1);
	starBody->SetKinematic(true);
	m_pStar = pStarGo;


	//Particle 
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,1.f,0.f };
	settings.minSize = .1f;
	settings.maxSize = .5f;
	settings.minEnergy = .5f;
	settings.maxEnergy = 1.f;
	settings.minScale = .5f;
	settings.maxScale = 1.f;
	settings.minEmitterRadius = 0.1f;
	settings.maxEmitterRadius = 1.5f;
	settings.color = { 1.f,1.f,1.f, .6f };
	auto pEmitter = m_pStar->AddComponent(new ParticleEmitterComponent(L"Textures/StarSparkle.png", settings, 10));
	m_pStar->SetParticle(pEmitter);
	pEmitter->SetActive(true);
	m_pStar->SetTag(L"Star");
}

void MyGameScene::CreateCoins()
{
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	auto pCoinGo = new Coin();
	auto pCoinGo1 = new Coin();
	auto pCoinGo2 = new Coin();

	//Particle 
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,1.f,0.f };
	settings.minSize = .1f;
	settings.maxSize = .5f;
	settings.minEnergy = .5f;
	settings.maxEnergy = 1.f;
	settings.minScale = .5f;
	settings.maxScale = 1.f;
	settings.minEmitterRadius = 0.1f;
	settings.maxEmitterRadius = .3f;
	settings.color = { 1.f,1.f,1.f, .6f };
	auto pEmitter = pCoinGo->AddComponent(new ParticleEmitterComponent(L"Textures/StarSparkle.png", settings, 10));
	pCoinGo->SetParticle(pEmitter);
	pEmitter->SetActive(true);
	pCoinGo->SetTag(L"Coin");

	pCoinGo->GetTransform()->Translate(m_OriginalCoinPosition1);
	AddChild(pCoinGo);
	auto coinBody = pCoinGo->AddComponent(new RigidBodyComponent());
	coinBody->AddCollider(PxBoxGeometry{ 0.2f, .2f, .2f }, *pDefaultMaterial, true);
	pCoinGo->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	coinBody->SetCollisionGroup(CollisionGroup::Group1);
	coinBody->SetKinematic(true);
	m_pCoin = pCoinGo;

	pCoinGo1->SetParticle(pEmitter);
	pEmitter->SetActive(true);
	pCoinGo1->SetTag(L"Coin");

	pCoinGo1->GetTransform()->Translate(m_OriginalCoinPosition2);
	AddChild(pCoinGo1);
	auto coinBody1 = pCoinGo1->AddComponent(new RigidBodyComponent());
	coinBody1->AddCollider(PxBoxGeometry{ 0.2f, .2f, .2f }, *pDefaultMaterial, true);
	pCoinGo1->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	coinBody1->SetCollisionGroup(CollisionGroup::Group1);
	coinBody1->SetKinematic(true);
	m_pCoin1 = pCoinGo1;


	pCoinGo2->SetParticle(pEmitter);
	pEmitter->SetActive(true);
	pCoinGo2->SetTag(L"Coin");

	pCoinGo2->GetTransform()->Translate(m_OriginalCoinPosition2);
	AddChild(pCoinGo2);
	auto coinBody2 = pCoinGo2->AddComponent(new RigidBodyComponent());
	coinBody2->AddCollider(PxBoxGeometry{ 0.2f, .2f, .2f }, *pDefaultMaterial, true);
	pCoinGo2->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	coinBody2->SetCollisionGroup(CollisionGroup::Group1);
	coinBody2->SetKinematic(true);
	m_pCoin2 = pCoinGo2;




}
void MyGameScene::CreateKoopaTroopas()
{
	const auto pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	const auto pKoopaGo = new KoopaTroopa();
	auto koopaBody = pKoopaGo->AddComponent(new RigidBodyComponent());
	koopaBody->AddCollider(PxBoxGeometry{ .35f, .75f, .75f }, *pDefaultMaterial, true, physx::PxTransform{ 0.f, -0.5f, -0.5 });
	pKoopaGo->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	//koopaBody->SetKinematic(true);
	pKoopaGo->GetTransform()->Translate(m_OriginalKoopaPosition);
	koopaBody->SetConstraint(RigidBodyConstraint::TransY, false);

	m_pKoopaTroopa = pKoopaGo;
	m_pKoopaTroopa->SetTag(L"KoopaTroopa");
	m_KillKoopaTroopa = false;
	AddChild(m_pKoopaTroopa);


	const auto pKoopaGo1 = new KoopaTroopa();
	auto koopaBody1 = pKoopaGo1->AddComponent(new RigidBodyComponent());
	koopaBody1->AddCollider(PxBoxGeometry{ .35f, .75f, .75f }, *pDefaultMaterial, true, physx::PxTransform{ 0.f, -0.5f, -0.5 });
	pKoopaGo1->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	//koopaBody->SetKinematic(true);
	pKoopaGo1->GetTransform()->Translate(m_OriginalKoopaPosition1);
	koopaBody1->SetConstraint(RigidBodyConstraint::TransY, false);

	m_pKoopaTroopa1 = pKoopaGo1;
	m_pKoopaTroopa1->SetTag(L"KoopaTroopa");
	AddChild(m_pKoopaTroopa1);


	const auto pKoopaGo2 = new KoopaTroopa();
	auto koopaBody2 = pKoopaGo2->AddComponent(new RigidBodyComponent());
	koopaBody2->AddCollider(PxBoxGeometry{ .35f, .75f, .75f }, *pDefaultMaterial, true, physx::PxTransform{ 0.f, -0.5f, -0.5 });
	pKoopaGo2->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	//koopaBody->SetKinematic(true);
	pKoopaGo2->GetTransform()->Translate(m_OriginalKoopaPosition2);
	koopaBody2->SetConstraint(RigidBodyConstraint::TransY, false);

	m_pKoopaTroopa2 = pKoopaGo2;
	m_pKoopaTroopa2->SetTag(L"KoopaTroopa");
	AddChild(m_pKoopaTroopa2);

	const auto pKoopaGo3 = new KoopaTroopa();
	auto koopaBody3 = pKoopaGo3->AddComponent(new RigidBodyComponent());
	koopaBody3->AddCollider(PxBoxGeometry{ .35f, .75f, .75f }, *pDefaultMaterial, true, physx::PxTransform{ 0.f, -0.5f, -0.5 });
	pKoopaGo3->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	//koopaBody->SetKinematic(true);
	pKoopaGo3->GetTransform()->Translate(m_OriginalKoopaPosition3);
	koopaBody3->SetConstraint(RigidBodyConstraint::TransY, false);

	m_pKoopaTroopa3 = pKoopaGo3;
	m_pKoopaTroopa3->SetTag(L"KoopaTroopa");
	AddChild(m_pKoopaTroopa3);

	const auto pKoopaGo4 = new KoopaTroopa();
	auto koopaBody4 = pKoopaGo4->AddComponent(new RigidBodyComponent());
	koopaBody4->AddCollider(PxBoxGeometry{ .35f, .75f, .75f }, *pDefaultMaterial, true, physx::PxTransform{ 0.f, -0.5f, -0.5 });
	pKoopaGo4->SetOnTriggerCallBack(std::bind(&MyGameScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//koopaBody->SetKinematic(true);
	pKoopaGo4->GetTransform()->Translate(m_OriginalKoopaPosition4);
	koopaBody4->SetConstraint(RigidBodyConstraint::TransY, false);

	m_pKoopaTroopa4 = pKoopaGo4;
	m_pKoopaTroopa4->SetTag(L"KoopaTroopa");
	AddChild(m_pKoopaTroopa4);


}
void MyGameScene::Update()
{
	//if (InputManager::IsKeyboardKey(InputState::pressed, VK_CONTROL))
	//{
	//	const auto pCameraTransform = m_SceneContext.pCamera->GetTransform();
	//	m_SceneContext.pLights->SetDirectionalLight(pCameraTransform->GetPosition(), { pCameraTransform->GetForward().x,pCameraTransform->GetForward().y, pCameraTransform->GetForward().z });
	//}
	TextRenderer::Get()->DrawText(m_pFont, StringUtil::utf8_decode(std::to_string(m_NrCoins)), m_TextPos, XMFLOAT4{ 1.f,1.f, 1.f, 1.f });
	m_pPauseMenu->GetTexture()->SetDimenson({ m_SceneContext.windowWidth, m_SceneContext.windowHeight });
	m_pDeathScreen->GetTexture()->SetDimenson({ m_SceneContext.windowWidth, m_SceneContext.windowHeight });
	m_pWinScreen->GetTexture()->SetDimenson({ m_SceneContext.windowWidth, m_SceneContext.windowHeight });
	if (!m_SceneInitialized)
	{
		m_PixelationTimer += m_SceneContext.pGameTime->GetElapsed();
		if (m_pPixelation->GetNrPixels() < 250)
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
	if (InputManager::IsKeyboardKey(InputState::pressed, VK_ESCAPE) && !m_pDeathScreen->IsEnabled() && !m_pWinScreen->IsEnabled())
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
		SoundManager::Get()->GetSystem()->playSound(m_pPauseSound, m_pSoundEffectGroup, false, nullptr);
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
	if (m_KillKoopaTroopa)
	{
		++m_KoopasKilled;
		m_KillKoopaTroopa = false;
		m_pCharacter->AddForce(0.f, 30.f, 0.f);
		m_pObjectToKill->GetTransform()->Translate(0.f, 0.f, 0.f);
	}
	if (!m_pUI[0]->GetComponent<SpriteComponent>()->IsEnabled() && !m_pDeathScreen->IsEnabled() && !m_pWinScreen->IsEnabled())
	{

		SoundManager::Get()->GetSystem()->playSound(m_pGameOverSound, m_pSoundEffectGroup, false, nullptr);
		m_pDeathScreen->Enable(true);
		//ResetScene();
	}
	if (m_pDeathScreen->IsEnabled() || m_pWinScreen->IsEnabled())
	{
		if (InputManager::IsKeyboardKey(InputState::pressed, VK_ESCAPE))
		{
			m_pDeathScreen->Enable(false);
			m_pWinScreen->Enable(false);
			ResetScene();

			SceneManager::Get()->PreviousScene();

		}
		if (InputManager::IsKeyboardKey(InputState::pressed, VK_SPACE))
		{
			m_pDeathScreen->Enable(false);
			m_pWinScreen->Enable(false);

			ResetScene();

		}

	}
	if (m_KoopasKilled >= 5)
	{
		m_pStar->GetTransform()->Translate(m_OriginalStarPosition);
		m_KoopasKilled = 0;

	}


}

void MyGameScene::OnGUI()
{
	//	ImGui::Checkbox("Draw ShadowMap", &m_DrawShadowMap);
	ImGui::SliderFloat("ShadowMap Scale", &m_ShadowMapScale, 0.f, 1.f);
}

void MyGameScene::OnTriggerCallBack(GameObject* pTriggerObject, GameObject* pOtherObject, PxTriggerAction action)
{

	if (pTriggerObject->GetTag() == L"Star" && pOtherObject->GetTag() == L"Mario")
	{
		m_pWinScreen->Enable(true);
	}
	if (pTriggerObject->GetTag() == L"Coin" && pOtherObject->GetTag() == L"Mario" && action == PxTriggerAction::ENTER)
	{
		SoundManager::Get()->GetSystem()->playSound(m_pCoinSound, m_pSoundEffectGroup, false, nullptr);
		m_NrCoins++;
		bool healthEnabled = false;
		for (int i = 0; i < 6; ++i)
		{
			if (!m_pUI[i]->GetComponent<SpriteComponent>()->IsEnabled() && !healthEnabled)
			{
				m_pUI[i]->GetComponent<SpriteComponent>()->Enable(true);
				healthEnabled = true;
			}

		}
		pTriggerObject->GetTransform()->Translate(0.f, 0.f, 0.f);
	}
	if (pTriggerObject->GetTag() == L"KoopaTroopa" && pOtherObject->GetTag() == L"Mario" && action == PxTriggerAction::ENTER)
	{
		if (pTriggerObject->GetTransform()->GetWorldPosition().y > pOtherObject->GetTransform()->GetWorldPosition().y)
		{
			SoundManager::Get()->GetSystem()->playSound(m_pOofSound, m_pSoundEffectGroup, false, nullptr);
			int amountDisabled = 0;
			for (int i = 5; i >= 0; --i)
			{
				if (m_pUI[i]->GetComponent<SpriteComponent>()->IsEnabled() && amountDisabled != 2)
				{
					++amountDisabled;
					m_pUI[i]->GetComponent<SpriteComponent>()->EnableWithDelay(false, float(amountDisabled));
				}

			}
		}
		else
		{
			m_KillKoopaTroopa = true;
			dynamic_cast<KoopaTroopa*>(pTriggerObject)->SetDead(true);
			m_pObjectToKill = pTriggerObject;
		}
	}
}


void MyGameScene::ResetScene()
{
	m_pCharacter->GetTransform()->Translate(m_OriginalPosition);
	m_pPauseMenu->Enable(false);
	m_pCharacter->SetPaused(false);
	m_pDeathScreen->Enable(false);
	m_pWinScreen->Enable(false);

	m_pKoopaTroopa->GetTransform()->Translate(m_OriginalKoopaPosition);
	m_pKoopaTroopa1->GetTransform()->Translate(m_OriginalKoopaPosition1);
	m_pKoopaTroopa2->GetTransform()->Translate(m_OriginalKoopaPosition2);
	m_pKoopaTroopa3->GetTransform()->Translate(m_OriginalKoopaPosition3);
	m_pKoopaTroopa4->GetTransform()->Translate(m_OriginalKoopaPosition4);

	m_pStar->GetTransform()->Translate(0.f, -10.f, 0.f);
	m_pCoin->GetTransform()->Translate(m_OriginalCoinPosition1);
	m_KoopasKilled = 0;
	m_NrCoins = 0;
	for (int i = 0; i < 6; ++i)
	{
		m_pUI[i]->GetComponent<SpriteComponent>()->Enable(true);
	}
}
void MyGameScene::PostDraw()
{
	//ShadowMapRenderer::Get()->Debug_DrawDepthSRV({ m_SceneContext.windowWidth - 10.f, 10.f }, { m_ShadowMapScale, m_ShadowMapScale }, { 1.f,0.f });
}