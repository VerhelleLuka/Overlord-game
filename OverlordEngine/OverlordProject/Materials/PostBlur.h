#pragma once
//Resharper Disable All
#include "Misc/PostProcessingMaterial.h"
#include <Base/Structs.h>
class PostBlur : public PostProcessingMaterial
{
public:
	PostBlur();
	~PostBlur() override = default;
	PostBlur(const PostBlur& other) = delete;
	PostBlur(PostBlur&& other) noexcept = delete;
	PostBlur& operator=(const PostBlur& other) = delete;
	PostBlur& operator=(PostBlur&& other) noexcept = delete;

	void IncreaseBlur();
	void DecreaseBlur();

	float GetBluriness() const { return m_Bluriness; }

protected:
	void Initialize(const GameContext& /*gameContext*/) override {}

private:
	float m_Bluriness = 5.f;
	
};

#pragma once
