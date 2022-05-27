#include "stdafx.h"
#include "stdafx.h"
#include "Goomba.h"
#include <Materials/ColorMaterial.h>
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>

void Goomba::Initialize(const SceneContext&)
{
	GetTransform()->Scale(0.1f);
	const auto pGoombaObject = new GameObject;
	auto pGoombaMesh = pGoombaObject->AddComponent(new ModelComponent(L"Meshes/Goomba.ovm"));
	pGoombaMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	pGoombaObject->GetTransform()->Scale(0.1f);
	AddChild(pGoombaObject);

	//auto pGoombaMesh = AddComponent(new ModelComponent(L"Meshes/Goomba.ovm"));
	//pGoombaMesh->SetMaterial(MaterialManager::Get()->CreateMaterial<ColorMaterial>());

	m_pEyesOpen = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pEyesOpen->SetDiffuseTexture(L"Textures/Goomba.bmp");

	m_pEyesClosed = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pEyesClosed->SetDiffuseTexture(L"Textures/Goomba_eyesclosed.png");


	const auto m_pBody  = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pBody->SetDiffuseTexture(L"Textures/Goomba_body.bmp");
	pGoombaMesh->SetMaterial(m_pEyesClosed, 3);
	pGoombaMesh->SetMaterial(m_pBody, 0);

	AddComponent(pGoombaMesh);


}

void Goomba::Update(const SceneContext&)
{

}