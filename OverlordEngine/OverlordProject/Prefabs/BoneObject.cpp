#include "stdafx.h"
#include "BoneObject.h"

BoneObject::BoneObject(BaseMaterial* pMaterial, float length = 5.f)
	:m_pMaterial(pMaterial)
	, m_Length(length)
{

}


void BoneObject::Initialize(const SceneContext&)
{
	GameObject* pEmpty = new GameObject();

	ModelComponent* pModel = new ModelComponent(L"Meshes/Bone.ovm");
	pModel->SetMaterial(m_pMaterial);

	pEmpty->AddComponent(pModel);
	pEmpty->GetTransform()->Rotate(0.f, -90.f, 0.f);

	pEmpty->GetTransform()->Scale(m_Length, m_Length, m_Length);
	AddChild(pEmpty);
}

void BoneObject::AddBone(BoneObject* pBone)
{
	pBone->GetTransform()->Translate(GetTransform()->GetPosition().x +  m_Length, GetTransform()->GetPosition().y, GetTransform()->GetPosition().z);
	AddChild(pBone);
}

void BoneObject::CalculateBindPose()
{
	XMFLOAT4X4 world = GetTransform()->GetWorld();
	//Convert from float4x4 to matrix
	XMMATRIX worldMat = XMLoadFloat4x4(&world);

	//Get determinant to calculate inverse of matrix
	XMVECTOR determinant = XMMatrixDeterminant(worldMat);
	
	//Calculate inverse
	XMMATRIX worldInverse = XMMatrixInverse(&determinant, worldMat);

	XMStoreFloat4x4(&m_BindPose, worldInverse);

	for (BoneObject* children : GetChildren<BoneObject>())
	{
		children->CalculateBindPose();
	}
}