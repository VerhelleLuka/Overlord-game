#include "stdafx.h"
#include "KoopaTroopa.h"
#include "Materials/DiffuseMaterial_Skinned.h"
#include <Materials/ColorMaterial.h>
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
KoopaTroopa::KoopaTroopa()
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
}

