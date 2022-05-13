#include "stdafx.h"
#include "SpikyScene.h"
#include "Components/ModelComponent.h"
#include <Materials/ColorMaterial.h>
#include "Prefabs/SpherePrefab.h"

SpikyScene::SpikyScene()
	:GameScene(L"SpikyScene")
{

}
void SpikyScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;
	//auto pBouncyMaterial = PxGetPhysics().createMaterial(.5f, .5f, .7f);
	m_SceneContext.settings.drawGrid = false;

	m_pSphere = new GameObject();

	//Material
	m_pSpikyMaterial = MaterialManager::Get()->CreateMaterial<SpikyMaterial>();
	auto pSphereMesh = new ModelComponent(L"Meshes/Octasphere.ovm");

	pSphereMesh->SetMaterial(m_pSpikyMaterial);
	m_pSphere->GetTransform()->Scale(XMFLOAT3{ 20.f,20.f,20.f });
	m_pSphere->AddComponent(pSphereMesh);
	AddChild(m_pSphere);
}

void SpikyScene::Update()
{
}

void SpikyScene::OnGUI()
{
	m_pSpikyMaterial->DrawImGui();
}