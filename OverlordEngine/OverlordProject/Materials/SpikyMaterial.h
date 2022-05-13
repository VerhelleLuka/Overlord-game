#pragma once
#include "Misc/Material.h"
#include "Misc/TextureData.h"
#include "Managers/ContentManager.h"
#include <string>
class SpikyMaterial final : public Material<SpikyMaterial>
{
public:
	SpikyMaterial();
	~SpikyMaterial() = default;

	SpikyMaterial(const SpikyMaterial& other) = delete;
	SpikyMaterial(SpikyMaterial&& other) noexcept = delete;
	SpikyMaterial& operator=(const SpikyMaterial& other) = delete;
	SpikyMaterial& operator=(SpikyMaterial&& other) noexcept = delete;


protected:
	void InitializeEffectVariables() override;

private:

};
