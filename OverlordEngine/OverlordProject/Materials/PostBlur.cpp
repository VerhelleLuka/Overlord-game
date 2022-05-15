//Resharper Disable All

#include "stdafx.h"
#include "PostBlur.h"

PostBlur::PostBlur() :
	PostProcessingMaterial(L"Effects/Post/Blur.fx")
{
}

void PostBlur::IncreaseBlur()
{
	
	m_Bluriness+= 0.01f;
	if (m_Bluriness >= 1.f)
		m_Bluriness = 1.f;
	m_pBaseEffect->GetVariableByName("gBluriness")->AsScalar()->SetFloat(m_Bluriness);
}

void PostBlur::DecreaseBlur()
{
	m_Bluriness -= 0.01f;
	
	if (m_Bluriness < 0.f)
		m_Bluriness = 0.f;
	m_pBaseEffect->GetVariableByName("gBluriness")->AsScalar()->SetFloat(m_Bluriness);
}
