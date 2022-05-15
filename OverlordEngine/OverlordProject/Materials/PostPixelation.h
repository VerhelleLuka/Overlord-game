#pragma once
//Resharper Disable All
#include "Misc/PostProcessingMaterial.h"
#include <Base/Structs.h>
class PostPixelation : public PostProcessingMaterial
{
public:
	PostPixelation();
	~PostPixelation() override = default;
	PostPixelation(const PostPixelation& other) = delete;
	PostPixelation(PostPixelation&& other) noexcept = delete;
	PostPixelation& operator=(const PostPixelation& other) = delete;
	PostPixelation& operator=(PostPixelation&& other) noexcept = delete;

	void IncreasePixelation();
	void DecreasePixelation();
	void SetNrPixels(int nr);
	int GetNrPixels() const { return m_NrPixels; }

protected:
	void Initialize(const GameContext& /*gameContext*/) override;
	
private:
	int m_NrPixels;
	ID3DX11EffectVariable* m_NrPixelsEffect;


};

#pragma once
#pragma once
