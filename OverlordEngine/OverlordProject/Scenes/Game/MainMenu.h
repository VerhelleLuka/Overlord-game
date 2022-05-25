#pragma once

class MainMenu : public GameScene
{
public:
	MainMenu() :GameScene(L"MainMenu") {}
	~MainMenu() override = default;
	MainMenu(const MainMenu& other) = delete;
	MainMenu(MainMenu&& other) noexcept = delete;
	MainMenu& operator=(const MainMenu& other) = delete;
	MainMenu& operator=(MainMenu&& other) noexcept = delete;

protected:
	void Initialize() override;
	void OnGUI() override;
	virtual void Update() override;

private:
	SpriteComponent* m_pTitleScreen;
};

