#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent() :
	m_FarPlane(2500.0f),
	m_NearPlane(0.1f),
	m_FOV(XM_PIDIV4),
	m_Size(25.0f),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, XMMatrixIdentity());
}

void CameraComponent::Update(const SceneContext& sceneContext)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	XMMATRIX projection{};

	if (m_PerspectiveProjection)
	{
		projection = XMMatrixPerspectiveFovLH(m_FOV, sceneContext.aspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * sceneContext.aspectRatio : sceneContext.windowWidth;
		const float viewHeight = (m_Size > 0) ? m_Size : sceneContext.windowHeight;
		projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	const XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const XMMATRIX view = XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::SetActive(bool active)
{
	if (m_IsActive == active) return;

	const auto pGameObject = GetGameObject();
	ASSERT_IF(!pGameObject, L"Failed to set active camera. Parent game object is null");

	if (!pGameObject) return; //help the compiler... (C6011)
	const auto pScene = pGameObject->GetScene();
	ASSERT_IF(!pScene, L"Failed to set active camera. Parent game scene is null");

	m_IsActive = active;
	pScene->SetActiveCamera(active?this:nullptr); //Switch to default camera if active==false
}

GameObject* CameraComponent::Pick( CollisionGroup /*ignoreGroups*/) const
{
	using namespace DirectX;

	DirectX::XMFLOAT2 ndc;
	GameContext gameContext;
	//Get window height width and mouse position
	UINT windowSettingsX = gameContext.windowWidth;
	UINT windowSettingsY = gameContext.windowHeight;
	const auto mousePosition = InputManager::GetMousePosition();

	const int width = windowSettingsX / 2;
	const int height = windowSettingsY / 2;

	//Convert from mouse to ndc coords
	ndc.x = (static_cast<float>(mousePosition.x) - width) / width;
	ndc.y = (height - static_cast<float>(mousePosition.y)) / height;

	//Beginning of the ray is the near point

	//Calculate near and far point
	auto n = XMFLOAT4(ndc.x, ndc.y, 0.f, 1.0f);
	auto f = XMFLOAT4(ndc.x, ndc.y, 1.f, 1.0f);
	XMMATRIX inverse{ XMLoadFloat4x4(&m_ViewProjectionInverse) };

	XMVECTOR nearPoint = XMVector3TransformCoord(XMLoadFloat4(&n), inverse);

	XMVECTOR farPoint = XMVector3TransformCoord(XMLoadFloat4(&f), inverse);

	const physx::PxVec3 rayStart = physx::PxVec3(XMVectorGetX( nearPoint), XMVectorGetY(nearPoint), XMVectorGetZ(nearPoint));

	const auto xmDirection = farPoint-nearPoint;

	physx::PxVec3 rayDir = physx::PxVec3(DirectX::XMVectorGetX(xmDirection), DirectX::XMVectorGetY(xmDirection),
		DirectX::XMVectorGetZ(xmDirection));
	rayDir.normalize();

	auto gameObject = GetGameObject();
	const auto gameScene = gameObject->GetScene();
	auto physXProxy = gameScene->GetPhysxProxy();
	//Just pick one million for maximum ray value
	const float maxRayDistance = 1000000;

	physx::PxRaycastBuffer hit;
	if (physXProxy->Raycast(rayStart, rayDir, maxRayDistance , hit, physx::PxHitFlag::eDEFAULT))
	{
		const auto actor = hit.block.actor;
		if (actor != nullptr)
		{
			const auto userData = actor->userData;
			if (userData != nullptr)
			{
				return reinterpret_cast<BaseComponent*>(userData)->GetGameObject();
			}
		}
	}
	return nullptr;
}