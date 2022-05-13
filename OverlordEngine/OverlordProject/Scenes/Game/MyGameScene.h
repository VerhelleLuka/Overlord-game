#pragma once
class Mario;
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

private:
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


};

