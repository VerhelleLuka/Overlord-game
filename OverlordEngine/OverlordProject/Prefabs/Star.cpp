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

	
}

void Star::Update(const SceneContext&)
{

}