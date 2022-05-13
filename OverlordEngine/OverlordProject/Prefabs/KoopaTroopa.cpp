#include "stdafx.h"
#include "KoopaTroopa.h"
#include "Materials/DiffuseMaterial_Skinned.h"
#include <Materials/ColorMaterial.h>

KoopaTroopa::KoopaTroopa()
{}

void KoopaTroopa::Initialize(const SceneContext& /*sceneContext*/)
{
	//Mesh
	const auto pKoopaObject = new GameObject;
	auto pKoopaMesh = pKoopaObject->AddComponent(new ModelComponent(L"Meshes/KoopaTroopa.ovm"));
	pKoopaMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	m_pModelComponent = pKoopaMesh;
	AddChild(pKoopaObject);

	//Texture
	//const auto pSkinnedMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	//pSkinnedMaterial->SetDiffuseTexture(L"Textures/Koopa_grp.png");

	//m_pModelComponent->SetMaterial(pSkinnedMaterial);
}

void KoopaTroopa::Update(const SceneContext& /*sceneContext*/)
{
}

