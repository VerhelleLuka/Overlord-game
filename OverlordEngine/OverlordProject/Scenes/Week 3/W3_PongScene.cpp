#include "stdafx.h"
#include "W3_PongScene.h"
#include "Prefabs/SpherePrefab.h"
PongScene::PongScene()
	:GameScene(L"PongScene")
{

}
void PongScene::Initialize()
{
	m_SceneContext.settings.enableOnGUI = true;

	auto pBouncyMaterial = PxGetPhysics().createMaterial(.0f, 0.f, 1.05f);
	GameSceneExt::CreatePhysXGroundPlane(*this, pBouncyMaterial);

	//Sphere 1
	m_pSphere = AddChild(new SpherePrefab(1.f, 10, XMFLOAT4{ Colors::Red }));
	auto pSphereActor = m_pSphere->AddComponent(new RigidBodyComponent());
	pSphereActor->SetConstraint(RigidBodyConstraint::TransY, false);
	pSphereActor->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);

	m_pSphere->GetTransform()->Translate(0.f, 0.f, -80.f);
	m_InitialBallPos = { 0.f, 0.f, -80.f };

	//Paddles
	m_pPaddle1 = AddChild(new CubePrefab(XMFLOAT3{ 1.f,1.f,5.f }, XMFLOAT4{ Colors::HotPink }));
	auto pPaddle1Actor = m_pPaddle1->AddComponent(new RigidBodyComponent());
	pPaddle1Actor->SetConstraint(RigidBodyConstraint::TransY, false);
	pPaddle1Actor->AddCollider(PxBoxGeometry{ .5f,.5f,2.5f }, *pBouncyMaterial);
	pPaddle1Actor->SetKinematic(true);
	m_pPaddle1->GetTransform()->Translate(-20.f, 0.f, -80.f);
	m_InitialPaddle1Pos = { -20.f, 0.f, -80.f };

	m_pPaddle2 = AddChild(new CubePrefab(XMFLOAT3{ 1.f,1.f,5.f }, XMFLOAT4{ Colors::PaleGoldenrod }));
	auto pPaddle2Actor = m_pPaddle2->AddComponent(new RigidBodyComponent());
	pPaddle2Actor->SetConstraint(RigidBodyConstraint::TransY, false);
	pPaddle2Actor->AddCollider(PxBoxGeometry{ .5f,.5f,2.5f }, *pBouncyMaterial);
	pPaddle2Actor->SetKinematic(true);
	m_pPaddle2->GetTransform()->Translate(20.f, 0.f, -80.f);
	m_InitialPaddle2Pos = { 20.f, 0.f, -80.f };

	//Camera pos and rotation
	reinterpret_cast<FreeCamera*>(m_SceneContext.pCamera->GetGameObject())->SetRotation(90, 0);


	//Edges
	m_pEdgeTop = AddChild(new CubePrefab(XMFLOAT3{ 100.f, 1.f, 1.f }));
	auto pRigidBodyTop = m_pEdgeTop->AddComponent(new RigidBodyComponent());
	pRigidBodyTop->AddCollider(PxBoxGeometry{ 50.f,.5f,.5f }, *pBouncyMaterial, false);
	pRigidBodyTop->SetKinematic(true);

	m_pEdgeTop->AddComponent(pRigidBodyTop);
	m_pEdgeTop->GetTransform()->Translate(0.f, 0.f, -60.f);

	m_pEdgeBottom = AddChild(new CubePrefab(XMFLOAT3{ 100.f, 1.f, 1.f }));
	auto pRigidBodyBottom = m_pEdgeBottom->AddComponent(new RigidBodyComponent());
	pRigidBodyBottom->AddCollider(PxBoxGeometry{ 50.f,.5f,.5f }, *pBouncyMaterial, false);
	pRigidBodyBottom->SetKinematic(true);

	m_pEdgeBottom->AddComponent(pRigidBodyBottom);
	m_pEdgeBottom->GetTransform()->Translate(0.f, 0.f, -100.f);

	//Triggers
	m_pTriggerLeft = AddChild(new CubePrefab(XMFLOAT3{ 1.f,1.f, 100.f }));
	auto pTriggerLeft = m_pTriggerLeft->AddComponent(new RigidBodyComponent());
	pTriggerLeft->AddCollider(PxBoxGeometry{ .5f, .5f, 50.f }, *pBouncyMaterial, true);
	pTriggerLeft->SetKinematic(true);
	m_pTriggerLeft->AddComponent(pTriggerLeft);
	m_pTriggerLeft->SetOnTriggerCallBack(std::bind(&PongScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_pTriggerLeft->GetTransform()->Translate(-35.f, 0.f, -80.f);

	m_pTriggerRight = AddChild(new CubePrefab(XMFLOAT3{ 1.f,1.f, 100.f }));
	auto pTriggerRight = m_pTriggerRight->AddComponent(new RigidBodyComponent());
	pTriggerRight->AddCollider(PxBoxGeometry{ .5f, .5f, 50.f }, *pBouncyMaterial, true);
	pTriggerRight->SetKinematic(true);
	m_pTriggerRight->AddComponent(pTriggerRight);
	m_pTriggerRight->SetOnTriggerCallBack(std::bind(&PongScene::OnTriggerCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_pTriggerRight->GetTransform()->Translate(35.f, 0.f, -80.f);
}

void PongScene::Update()
{
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, VK_SPACE) && !m_GameStarted)
	{
		m_pSphere->GetComponent<RigidBodyComponent>()->AddForce(XMFLOAT3{ -1000.f, 0.f, 400.f });
		m_GameStarted = true;
	}
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, VK_UP))
	{
		auto paddlePos = m_pPaddle2->GetTransform()->GetPosition();
		m_pPaddle2->GetTransform()->Translate(paddlePos.x, 0.f, paddlePos.z + (1.f * m_PaddleSpeed));
	}
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, VK_DOWN))
	{
		auto paddlePos = m_pPaddle2->GetTransform()->GetPosition();
		m_pPaddle2->GetTransform()->Translate(paddlePos.x, 0.f, paddlePos.z + (-1.f * m_PaddleSpeed));
	}
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, 0x54))//"T"
	{
		auto paddlePos = m_pPaddle1->GetTransform()->GetPosition();
		m_pPaddle1->GetTransform()->Translate(paddlePos.x, 0.f, paddlePos.z + (1.f * m_PaddleSpeed));
	}
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, 0x47))//"G"
	{
		auto paddlePos = m_pPaddle1->GetTransform()->GetPosition();
		m_pPaddle1->GetTransform()->Translate(paddlePos.x, 0.f, paddlePos.z + (-1.f * m_PaddleSpeed));
	}
}

void PongScene::OnTriggerCallBack(GameObject* /*pTriggerObject*/, GameObject* /*pOtherObject*/, PxTriggerAction /*action*/)
{
	m_pSphere->GetTransform()->Translate(m_InitialBallPos);
	m_pPaddle1->GetTransform()->Translate(m_InitialPaddle1Pos);
	m_pPaddle2->GetTransform()->Translate(m_InitialPaddle2Pos);
	m_GameStarted = false;
	

}