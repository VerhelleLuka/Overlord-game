#include "stdafx.h"
#include "ModelTestScene.h"
#include "Components/ModelComponent.h"
#include <Materials/ColorMaterial.h>
#include <Materials/DiffuseMaterial.h>
ModelTestScene::ModelTestScene()
	:GameScene(L"ModelTestScene")
{

}
void ModelTestScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;
	auto pBouncyMaterial = PxGetPhysics().createMaterial(.5f, .5f, .7f);
	GameSceneExt::CreatePhysXGroundPlane(*this, pBouncyMaterial);


	//CHAAAAAIR
	m_pChair = new GameObject();
	auto pChairMesh = new ModelComponent(L"Meshes/Chair.ovm");

	//Material
	ColorMaterial* pColorMaterial = MaterialManager::Get()->CreateMaterial<ColorMaterial>();
	DiffuseMaterial* pDiffuseMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	pDiffuseMaterial->SetDiffuseTexture(L"Textures/Chair_Dark.dds");
	pColorMaterial->SetVariable_Vector(L"gColor", XMFLOAT4{ Colors::Red });
	pChairMesh->SetMaterial(pDiffuseMaterial);
	
	const auto pChairRigidBody{ContentManager::Load<PxConvexMesh>(L"Meshes/Chair.ovpc") };
	
	auto pChairActor = m_pChair->AddComponent(new RigidBodyComponent(false));
	pChairActor->AddCollider(PxConvexMeshGeometry( pChairRigidBody ), *pBouncyMaterial);
	
	//========
	//Chaaair.


	m_pChair->AddComponent(pChairMesh);
	m_pChair->GetTransform()->Translate(1.f, 10.f, 0.f);
	AddChild(m_pChair);
}

void ModelTestScene::Update()
{
}

void ModelTestScene::OnGUI()
{

}