//Resharper Disable All

#include "stdafx.h"
#include "PostPixelation.h"

PostPixelation::PostPixelation() :
	PostProcessingMaterial(L"Effects/Post/Pixelation.fx")
{
	m_NrPixels = 2;
}

void PostPixelation::Initialize(const GameContext&)
{
	m_NrPixelsEffect = m_pBaseEffect->GetVariableByName("gNrPixels");

}
void PostPixelation::SetNrPixels(int nr)
{
	m_NrPixels = nr;
	m_pBaseEffect->GetVariableByName("gNrPixels")->AsScalar()->SetInt(nr);

}
void PostPixelation::IncreasePixelation()
{

	m_NrPixels++;

	m_pBaseEffect->GetVariableByName("gNrPixels")->AsScalar()->SetInt(m_NrPixels);
}

void PostPixelation::DecreasePixelation()
{
	m_NrPixels--;
	m_pBaseEffect->GetVariableByName("gNrPixels")->AsScalar()->SetInt(m_NrPixels);
}
