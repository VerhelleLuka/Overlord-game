#include "stdafx.h"
#include "Coin.h"
#include <Materials/ColorMaterial.h>
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>
#include <Materials/DiffuseMaterial.h>

void Coin::Initialize(const SceneContext&)
{
	auto pCoinMesh = AddComponent(new ModelComponent(L"Meshes/Coin.ovm"));
	pCoinMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	const auto pCoinMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pCoinMat->SetDiffuseTexture(L"Textures/Coin.png");


	pCoinMesh->SetMaterial(pCoinMat);

	AddComponent(pCoinMesh);

	m_Rotation = 0.f;
}

void Coin::Update(const SceneContext& sceneContext)
{
	GetTransform()->Rotate(0.f, m_Rotation, 0.f, true);
	m_Rotation += 60.f * sceneContext.pGameTime->GetElapsed();

	if (m_Rotation >= 360.f)
		m_Rotation = 0.f;


}