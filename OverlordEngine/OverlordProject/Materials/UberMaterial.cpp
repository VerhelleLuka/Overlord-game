#include "stdafx.h"
#include "UberMaterial.h"
#include "Managers/ContentManager.h"

UberMaterial::UberMaterial() :
	Material<UberMaterial>(L"Effects/UberShader.fx")
{
	InitializeEffectVariables();
}

void UberMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{     
	m_pUberTextureDiffuse = ContentManager::Load<TextureData>(assetFile);

	SetVariable_Texture(L"gTextureDiffuse", m_pUberTextureDiffuse);
}

void UberMaterial::SetNormalTexture(const std::wstring& assetFile)
{
	m_pUberTextureNormal = ContentManager::Load<TextureData>(assetFile);

	SetVariable_Texture(L"gTextureNormal", m_pUberTextureNormal);
}


void UberMaterial::InitializeEffectVariables()
{
	SetVariable_Scalar(L"gUseTextureDiffuse", true);
	SetVariable_Scalar(L"gUseTextureNormal", true);

	SetVariable_Vector(L"gColorDiffuse", XMFLOAT3{ Colors::Purple });

}
