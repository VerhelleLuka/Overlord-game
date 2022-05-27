#include "stdafx.h"
#include "Star.h"
#include <Materials/ColorMaterial.h>
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>
#include <Materials/DiffuseMaterial.h>

void Star::Initialize(const SceneContext&)
{
	auto pStarMesh = AddComponent(new ModelComponent(L"Meshes/Star.ovm"));
	pStarMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	const auto pStarMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pStarMat->SetDiffuseTexture(L"Textures/Star.png");

	const auto pStarEyeMat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	pStarEyeMat->SetDiffuseTexture(L"Textures/Star_eye.png");

	pStarMesh->SetMaterial(pStarMat, 1);
	pStarMesh->SetMaterial(pStarEyeMat, 0);

	AddComponent(pStarMesh);

	m_Rotation = 0.f;
}

void Star::Update(const SceneContext& sceneContext)
{
	GetTransform()->Rotate(0.f, m_Rotation , 0.f, true);
	m_Rotation += 60.f * sceneContext.pGameTime->GetElapsed();

	if (m_Rotation >= 360.f)
		m_Rotation = 0.f;

	
}