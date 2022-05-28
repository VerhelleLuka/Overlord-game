#include "stdafx.h"
#include "KoopaTroopa.h"
#include "Materials/DiffuseMaterial_Skinned.h"
#include <Materials/ColorMaterial.h>
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
KoopaTroopa::KoopaTroopa()
	:m_IsDead(false)
{}

void KoopaTroopa::Initialize(const SceneContext& /*sceneContext*/)
{
	//Mesh

	const auto pKoopaObject = new GameObject();
	auto pKoopaMesh = pKoopaObject->AddComponent(new ModelComponent(L"Meshes/KoopaTroopa.ovm"));
	pKoopaMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());
	pKoopaObject->GetTransform()->Translate(0.f, -100.f, 0.f);
	m_pModelComponent = pKoopaMesh;
	AddChild(pKoopaObject);

	//Texture
	const auto pSkinnedMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pSkinnedMaterial->SetDiffuseTexture(L"Textures/Koopa_grp.png");

	m_pModelComponent->SetMaterial(pSkinnedMaterial);

	GetTransform()->Scale(0.0125f);
}

void KoopaTroopa::Update(const SceneContext& /*sceneContext*/)
{
	//PxRaycastBuffer raycastBuffer;
	//PxVec3 origin = { GetTransform()->GetPosition().x,
	//GetTransform()->GetPosition().y + 0.25f  ,
	//GetTransform()->GetPosition().z };
	//PxVec3 direction = { GetTransform()->GetUp().x,
	// GetTransform()->GetUp().y,
	// GetTransform()->GetUp().z };

	//if (SceneManager::Get()->GetActiveScene()->GetPhysxProxy()->Raycast(origin, direction, .5f, raycastBuffer))
	//{
	//	GetComponent<RigidBodyComponent>()->SetConstraint(RigidBodyConstraint::TransY, true);
	//	GetComponent<RigidBodyComponent>()->SetKinematic(false);
	//	std::cout << "Hye\n";
	//}

}

