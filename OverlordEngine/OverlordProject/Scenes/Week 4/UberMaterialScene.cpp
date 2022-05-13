#include "stdafx.h"
#include "UberMaterialScene.h"
#include "Components/ModelComponent.h"
#include <Materials/ColorMaterial.h>
#include "Prefabs/SpherePrefab.h"

UberMaterialScene::UberMaterialScene()
	:GameScene(L"UberMaterialScene")
{

}
void UberMaterialScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;
	//auto pBouncyMaterial = PxGetPhysics().createMaterial(.5f, .5f, .7f);
	m_SceneContext.settings.drawGrid = false;

	m_pSphere = new GameObject();

	//Material
	 m_pUberMaterial = MaterialManager::Get()->CreateMaterial<UberMaterial>();
	 m_pUberMaterial->SetDiffuseTexture(L"Textures/Skulls_Diffusemap.tga");
	 m_pUberMaterial->SetNormalTexture(L"Textures/Skulls_Normalmap.tga");
	auto pSphereMesh = new ModelComponent(L"Meshes/Sphere.ovm");

	pSphereMesh->SetMaterial(m_pUberMaterial);
	m_pSphere->GetTransform()->Scale(XMFLOAT3{ 20.f,20.f,20.f});
	m_pSphere->AddComponent(pSphereMesh);
	AddChild(m_pSphere);
}

void UberMaterialScene::Update()
{
}

void UberMaterialScene::OnGUI()
{
	m_pUberMaterial->DrawImGui();
}