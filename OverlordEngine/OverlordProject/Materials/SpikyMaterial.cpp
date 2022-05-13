#include "stdafx.h"
#include "SpikyMaterial.h"
#include "Managers/ContentManager.h"

SpikyMaterial::SpikyMaterial() :
	Material<SpikyMaterial>(L"Effects/SpikyShader.fx")
{
	InitializeEffectVariables();
}

void SpikyMaterial::InitializeEffectVariables()
{
	SetVariable_Vector(L"gColorDiffuse", XMFLOAT3{ Colors::Purple });

}
