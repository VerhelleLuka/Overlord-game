#include "stdafx.h"
#include "SkyBox.h"

SkyBox::SkyBox() :
	Material<SkyBox>(L"Effects/Skybox.fx")
{}
void SkyBox::SetDiffuseTexture(const std::wstring& assetFile)
{
	auto difTex =  ContentManager::Load<TextureData>(assetFile);

	SetVariable_Texture(L"gTexture", difTex);
}
