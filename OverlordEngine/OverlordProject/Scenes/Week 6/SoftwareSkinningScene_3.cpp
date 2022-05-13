#include "stdafx.h"
#include "SoftwareSkinningScene_3.h"
#include "Prefabs/BoneObject.h"
#include "Materials/ColorMaterial.h"
void SoftwareSkinningScene_3::Initialize()
{

	m_SceneContext.settings.enableOnGUI = true;
	ColorMaterial* pMaterial = MaterialManager::Get()->CreateMaterial<ColorMaterial>();

	GameObject* pRoot = new GameObject();
	AddChild(pRoot);

	//Week 2 
	m_pMeshDrawer = new MeshDrawComponent(24, true);
	GameObject* pBoxDrawer = new GameObject();

	pBoxDrawer->AddComponent(m_pMeshDrawer);
	//
	m_pBone0 = new BoneObject(pMaterial, 15.f);
	m_pBone1 = new BoneObject(pMaterial, 15.f);
	m_pBone0->AddBone(m_pBone1);

	pRoot->AddChild(m_pBone0);

	m_pBone0->CalculateBindPose();

	AddChild(pBoxDrawer);

	InitializeVertices(15.f);
}

void SoftwareSkinningScene_3::Update()
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

	XMMATRIX bindPose0 = XMLoadFloat4x4(&m_pBone0->GetBindPose());
	XMMATRIX bindPose1 = XMLoadFloat4x4(&m_pBone1->GetBindPose());
	//Calculate bone transforms
	XMMATRIX boneTransform0 = XMMatrixMultiply(bindPose0, XMLoadFloat4x4(&m_pBone0->GetTransform()->GetWorld()));
	XMMATRIX boneTransform1 = XMMatrixMultiply(bindPose1, XMLoadFloat4x4(&m_pBone1->GetTransform()->GetWorld()));

	for (int i{}; i < m_SkinnedVertices.size(); ++i)
	{
		XMVECTOR vector = XMLoadFloat3(&m_SkinnedVertices[i].originalVertex.Position);

		//Transform the originalVertex position by the bone transform
		XMVECTOR weightedVertex0 = XMVector3TransformCoord(vector, boneTransform0);
		XMVECTOR weightedVertex1 = XMVector3TransformCoord(vector, boneTransform1);

		vector = XMVectorLerp(weightedVertex0, weightedVertex1, m_SkinnedVertices[i].blendWeight1);
		XMStoreFloat3(&m_SkinnedVertices[i].transformedVertex.Position, vector);

	}
	m_pMeshDrawer->RemoveTriangles();

	for (int i{}; i < m_SkinnedVertices.size(); i += 4)
	{
		QuadPosNormCol quad;
		quad.Vertex1 = m_SkinnedVertices[i].transformedVertex;
		quad.Vertex2 = m_SkinnedVertices[i + 1].transformedVertex;
		quad.Vertex3 = m_SkinnedVertices[i + 2].transformedVertex;
		quad.Vertex4 = m_SkinnedVertices[i + 3].transformedVertex;
		m_pMeshDrawer->AddQuad(quad);
	}
	m_pMeshDrawer->UpdateBuffer();


}

void SoftwareSkinningScene_3::OnGUI()
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

void SoftwareSkinningScene_3::InitializeVertices(float length)
{
	auto pos = XMFLOAT3(length / 2.f, 0.f, 0.f);
	const auto offset = XMFLOAT3(length / 2.f, 2.5f, 2.5f);
	auto col = XMFLOAT4(1.f, 0.f, 0.f, 0.5f);
#pragma region BOX 1
	//FRONT
	XMFLOAT3 norm = { 0, 0, -1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col,	 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col,	0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	//BACK
	norm = { 0, 0, 1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col,0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	//TOP
	norm = { 0, 1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	//BOTTOM
	norm = { 0, -1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	//LEFT
	norm = { -1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 1.0f, 0.0f);
	//RIGHT
	norm = { 1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
#pragma endregion

	col = { 0.f, 1.f, 0.f, 0.5f };
	pos = { 22.5f, 0.f, 0.f };
#pragma region BOX 2
	//FRONT
	norm = { 0, 0, -1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	//BACK
	norm = { 0, 0, 1 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	//TOP
	norm = { 0, 1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	//BOTTOM
	norm = { 0, -1, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	//LEFT
	norm = { -1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ -offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.5f, 0.5f);
	//RIGHT
	norm = { 1, 0, 0 };
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, offset.y + pos.y, offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, offset.z + pos.z }, norm, col, 0.0f, 1.0f);
	m_SkinnedVertices.emplace_back(XMFLOAT3{ offset.x + pos.x, -offset.y + pos.y, -offset.z + pos.z }, norm, col, 0.0f, 1.0f);
#pragma endregion
}