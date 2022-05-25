#pragma once
class PostPixelation;
class Mario;
class PostGrayscale;
class DiffuseMaterial_Shadow;
class ModelComponent;
class MyGameScene : public GameScene
{
public:
	MyGameScene() :GameScene(L"MyGameScene") {}
	~MyGameScene() override = default;
	MyGameScene(const MyGameScene& other) = delete;
	MyGameScene(MyGameScene&& other) noexcept = delete;
	MyGameScene& operator=(const MyGameScene& other) = delete;
	MyGameScene& operator=(MyGameScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void OnGUI() override;
	virtual void Update() override;

private:
	void CreateLevel();
	enum InputIds
	{
		CharacterMoveLeft,
		CharacterMoveRight,
		CharacterMoveForward,
		CharacterMoveBackward,
		CharacterJump,
		CharacterRun,
		CharacterDuck
	};


	Mario* m_pCharacter{};
	ParticleEmitterComponent* m_pEmitter;
	PostPixelation* m_pPixelation;
	PostGrayscale* m_pGrayscale;
	bool m_SceneInitialized;
	int m_NrPixels;
	float m_PixelationTimer = 0.f;
	const float m_PixelationTime = 0.001f;
	GameObject* m_pUI[6];
	SpriteComponent* m_pPauseMenu;

	const XMFLOAT3 m_OriginalPosition = {-30.f, 10.f, -30.f};
};

