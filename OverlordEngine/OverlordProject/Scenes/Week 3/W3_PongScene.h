#pragma once
#include "Prefabs/SpherePrefab.h"
#include "Prefabs/CubePrefab.h"
class PongScene final : public GameScene
{
public:
	PongScene();
	~PongScene() override = default;

	PongScene(const PongScene& other) = delete;
	PongScene(PongScene&& other) noexcept = delete;
	PongScene& operator=(const PongScene& other) = delete;
	PongScene& operator=(PongScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;

	void OnTriggerCallBack(GameObject* /*pTriggerObject*/, GameObject* /*pOtherObject*/, PxTriggerAction /*action*/);

private:
	SpherePrefab* m_pSphere;
	CubePrefab* m_pPaddle1;
	CubePrefab* m_pPaddle2;

	CubePrefab* m_pEdgeTop;
	CubePrefab* m_pEdgeBottom;

	CubePrefab* m_pTriggerLeft;
	CubePrefab* m_pTriggerRight;

	XMFLOAT3 m_InitialBallPos;
	XMFLOAT3 m_InitialPaddle1Pos;
	XMFLOAT3 m_InitialPaddle2Pos;

	bool m_GameStarted = false;


	const float m_PaddleSpeed = 0.3f;
};
