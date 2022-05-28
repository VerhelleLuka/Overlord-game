#include "stdafx.h"
#include "MainMenu.h"

void MainMenu::Initialize()
{
	m_SceneContext.settings.enableOnGUI = false;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.showInfoOverlay = false;
	m_SceneContext.pLights->SetDirectionalLight({ 0,66.1346436f,-21.1850471f }, { -0.740129888f, 0.597205281f, 0.309117377f });
	
	auto pGo = new GameObject();
	m_pTitleScreen = new SpriteComponent(L"Textures/TitleScreen.png");
	pGo->AddComponent(m_pTitleScreen);
	AddChild(pGo);
}

void MainMenu::Update()
{
	m_pTitleScreen->GetTexture()->SetDimenson({ m_SceneContext.windowWidth, m_SceneContext.windowHeight });

	if (InputManager::IsKeyboardKey(InputState::pressed, VK_SPACE))
	{
		SceneManager::Get()->NextScene();
	}
	if (InputManager::IsKeyboardKey(InputState::pressed, VK_ESCAPE))
	{
		PostQuitMessage(1);
	}
}

void MainMenu::OnGUI()
{

}