#include "stdafx.h"
#include "DiffuseMaterial.h"
#include "Managers/ContentManager.h"

DiffuseMaterial::DiffuseMaterial() :
	Material<DiffuseMaterial>(L"Effects/PosNormTex3D.fx")
{}

void DiffuseMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);

	SetVariable_Texture(L"gDiffuseMap", m_pDiffuseTexture);
}

void DiffuseMaterial::InitializeEffectVariables()
{

}
