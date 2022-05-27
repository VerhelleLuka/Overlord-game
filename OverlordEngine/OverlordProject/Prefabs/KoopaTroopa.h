#pragma once
#include <stdafx.cpp>
#include "Character.h"


class KoopaTroopa : public GameObject
{
public:
	KoopaTroopa();
	~KoopaTroopa() override = default;

	KoopaTroopa(const KoopaTroopa& other) = delete;
	KoopaTroopa(KoopaTroopa&& other) noexcept = delete;
	KoopaTroopa& operator=(const KoopaTroopa& other) = delete;
	KoopaTroopa& operator=(KoopaTroopa&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:

	ModelComponent* m_pModelComponent;
};

