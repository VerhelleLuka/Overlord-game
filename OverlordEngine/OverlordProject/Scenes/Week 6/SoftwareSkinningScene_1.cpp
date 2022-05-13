#include "stdafx.h"
#include "SoftwareSkinningScene_1.h"
#include "Prefabs/BoneObject.h"
#include "Materials/ColorMaterial.h"
void SoftwareSkinningScene_1::Initialize()
{
	
	m_SceneContext.settings.enableOnGUI = true;
	ColorMaterial* pMaterial = MaterialManager::Get()->CreateMaterial<ColorMaterial>();;
	pMaterial->SetVariable_Vector(L"gColor", XMFLOAT4{ Colors::Red });
	GameObject* pRoot = new GameObject();

	m_pBone0 = new BoneObject(pMaterial, 15.f);

	pRoot->AddChild(m_pBone0);

	m_pBone1 = new BoneObject(pMaterial, 15.f);

	m_pBone0->AddBone(m_pBone1);

	AddChild(pRoot);
}

void SoftwareSkinningScene_1::Update()
{
	if (!m_ManualRotation)
	{
		m_BoneRotation += 45 * m_RotationSign * m_SceneContext.pGameTime->GetElapsed();
		if (m_RotationSign > 0 && m_BoneRotation >= 45)
		{
			m_RotationSign = -1;

		}
		else if (m_RotationSign < 0 && m_BoneRotation <= -45.0f)
		{
			m_RotationSign = 1;
		}
		m_pBone0->GetTransform()->Rotate(0, 0, m_BoneRotation);
		m_pBone1->GetTransform()->Rotate(0, 0, -m_BoneRotation * 2.f);
	}

}

void SoftwareSkinningScene_1::OnGUI()
{
	if (ImGui::CollapsingHeader("Rotation stuff"))
	{
		ImGui::Checkbox("Manual rotation", &m_ManualRotation);
		const char* label = "Yes";
		float newRotation = { 0.f };
		float something[] = { newRotation, 0,0 };
		if (ImGui::SliderFloat3(label, something, -45.f, 45.f))
		{
			m_BoneRotation = something[0];
			m_pBone0->GetTransform()->Rotate(0, 0, m_BoneRotation);
			m_pBone1->GetTransform()->Rotate(0, 0, -m_BoneRotation * 2.f);
		}
	}
}