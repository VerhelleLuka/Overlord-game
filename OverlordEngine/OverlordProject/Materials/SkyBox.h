#pragma once
class SkyBox final: public Material<SkyBox>
{
public:
	SkyBox();
	~SkyBox() = default;
	SkyBox(const SkyBox& other) = delete;
	SkyBox(SkyBox&& other) noexcept = delete;
	SkyBox& operator=(const SkyBox& other) = delete;
	SkyBox& operator=(SkyBox&& other) noexcept = delete;
	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override {};
};

