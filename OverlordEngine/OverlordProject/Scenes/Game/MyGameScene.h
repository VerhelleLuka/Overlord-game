#pragma once
class PostPixelation;
class Mario;
class PostGrayscale;
class DiffuseMaterial_Shadow;
class ModelComponent;
class Star;
class KoopaTroopa;
class Coin;
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
	 void OnTriggerCallBack(GameObject* /*pTriggerObject*/, GameObject* /*pOtherObject*/, PxTriggerAction /*action*/) ;

private:
	void CreateLevel();
	void CreateStar();
	void CreateKoopaTroopas();
	void CreateCoins();
	void ResetScene();
	enum InputIdss
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
	Star* m_pStar;
	KoopaTroopa* m_pKoopaTroopa;
	KoopaTroopa* m_pKoopaTroopa1;
	KoopaTroopa* m_pKoopaTroopa2;
	KoopaTroopa* m_pKoopaTroopa3;
	KoopaTroopa* m_pKoopaTroopa4;
	GameObject* m_pCharacterRigidBody;
	ParticleEmitterComponent* m_pEmitter;
	PostPixelation* m_pPixelation;
	PostGrayscale* m_pGrayscale;
	bool m_SceneInitialized;
	int m_NrPixels;
	float m_PixelationTimer = 0.f;
	const float m_PixelationTime = 0.001f;
	GameObject* m_pUI[6];
	SpriteComponent* m_pPauseMenu;
	SpriteComponent* m_pDeathScreen;
	SpriteComponent* m_pWinScreen;
	const XMFLOAT3 m_OriginalPositionOG = {-30.f, 10.f, -30.f};
	const XMFLOAT3 m_OriginalPosition = { 15.f, 30.2f, 20.f };
	const XMFLOAT3 m_OriginalStarPosition = {-25.f, 10.f, -25.f};

	const XMFLOAT3 m_OriginalKoopaPosition = { 20.f, 13.5f, -10.f };
	const XMFLOAT3 m_OriginalKoopaPosition1 = { -30.f, 13.5f, -10.f };
	const XMFLOAT3 m_OriginalKoopaPosition2 = { 25.f, 25.f, -5.f };
	const XMFLOAT3 m_OriginalKoopaPosition3 = { 8.9f, 31.f, 19.f };
	const XMFLOAT3 m_OriginalKoopaPosition4 = { -29.f, 14.5f, 9.f };

	FMOD::Sound* m_pGameOverSound{};
	FMOD::Sound* m_pOofSound{};

	FMOD::Sound* m_pPauseSound{};
	FMOD::Sound* m_pStarSound{};
	FMOD::ChannelGroup* m_pSoundEffectGroup{};

	bool m_KillKoopaTroopa;
	const int m_NrKoopas = 5;
	int m_KoopasKilled;
	GameObject* m_pObjectToKill;

	Coin* m_pCoin;

	int m_ReallyEnableStar;//for some reason at the beginnin of the scene mario overlaps with the star TWICE, so this is a little counter to keep track of when mario actually
	//overlaps with the star

};

