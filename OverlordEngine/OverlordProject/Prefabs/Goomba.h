#pragma once
#pragma once
#include <stdafx.cpp>
class DiffuseMaterial_Shadow;
enum class GoombaState
{
	IDLE,
	WALKING
};

class Goomba : public GameObject
{
public:
	Goomba() = default;
	~Goomba() override = default;

	Goomba(const Goomba& other) = delete;
	Goomba(Goomba&& other) noexcept = delete;
	Goomba& operator=(const Goomba& other) = delete;
	Goomba& operator=(Goomba&& other) noexcept = delete;

	bool GetPaused() const { return m_IsPaused; }
	void SetPaused(bool isPaused) { m_IsPaused = isPaused; }

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	bool m_IsPaused;

	DiffuseMaterial_Shadow* m_pEyesOpen;
	DiffuseMaterial_Shadow* m_pEyesClosed;

};

