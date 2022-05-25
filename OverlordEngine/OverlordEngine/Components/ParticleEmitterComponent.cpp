#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include "Misc/ParticleMaterial.h"

ParticleMaterial* ParticleEmitterComponent::m_pParticleMaterial{};

ParticleEmitterComponent::ParticleEmitterComponent(const std::wstring& assetFile, const ParticleEmitterSettings& emitterSettings, UINT particleCount) :
	m_ParticlesArray(new Particle[particleCount]),
	m_ParticleCount(particleCount), //How big is our particle buffer?
	m_MaxParticles(particleCount), //How many particles to draw (max == particleCount)
	m_AssetFile(assetFile),
	m_EmitterSettings(emitterSettings)
	, m_LastParticleInit(0.f)
	, m_IsActive(false)
{
	m_enablePostDraw = true; //This enables the PostDraw function for the component
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	delete m_ParticlesArray;
	m_pVertexBuffer->Release();
}

void ParticleEmitterComponent::Initialize(const SceneContext& sceneContext)
{
	if (!m_pParticleMaterial)
		m_pParticleMaterial = MaterialManager::Get()->CreateMaterial<ParticleMaterial>();
	CreateVertexBuffer(sceneContext);
	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);

}

void ParticleEmitterComponent::CreateVertexBuffer(const SceneContext& sceneContext)
{
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(VertexParticle) * m_ParticleCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	sceneContext.d3dContext.pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer);
}
void ParticleEmitterComponent::SpawnNrOfParticles(int amount, const SceneContext& sceneContext)
{
	for (int i{}; i < amount; ++i)
	{
		Update(sceneContext);
	}
}
void ParticleEmitterComponent::Update(const SceneContext& sceneContext)
{
	float particleInterval = (m_EmitterSettings.maxEnergy - m_EmitterSettings.minEnergy) / m_ParticleCount;
	m_LastParticleInit += sceneContext.pGameTime->GetElapsed();

	m_ActiveParticles = 0;
	D3D11_MAPPED_SUBRESOURCE subMap;
	sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subMap);
	VertexParticle* pData = (VertexParticle*)subMap.pData;

	float deltaTime = sceneContext.pGameTime->GetElapsed();
	for (size_t i = 0; i < m_ParticleCount; ++i)
	{
		if (m_ParticlesArray[i].isActive)
		{
			UpdateParticle(m_ParticlesArray[i], deltaTime);
		}
		if (!m_ParticlesArray[i].isActive && m_LastParticleInit >= particleInterval && m_IsActive)
		{
			SpawnParticle(m_ParticlesArray[i]);
		}

		if (m_ParticlesArray[i].isActive)
		{
			pData[m_ActiveParticles] = m_ParticlesArray[i].vertexInfo;
			++m_ActiveParticles;
		}

	}
		sceneContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}
void ParticleEmitterComponent::UpdateParticle(Particle& p, float elapsedTime) const
{
	if (p.isActive)
	{
		p.currentEnergy -= elapsedTime;
		if (p.currentEnergy < 0)
		{
			p.isActive = false;
			return;
		}
		p.vertexInfo.Position.x += m_EmitterSettings.velocity.x * elapsedTime;
		p.vertexInfo.Position.y += m_EmitterSettings.velocity.y * elapsedTime;
		p.vertexInfo.Position.z += m_EmitterSettings.velocity.z * elapsedTime;

		float lifePercent = p.currentEnergy / p.totalEnergy;

		p.vertexInfo.Color.w = m_EmitterSettings.color.w * lifePercent;


		p.vertexInfo.Size = p.initialSize + (1 - lifePercent) * p.sizeChange;

	}
	else
		return;
}

void ParticleEmitterComponent::SpawnParticle(Particle& p)
{
	m_LastParticleInit = 0;

	p.isActive = true;
	p.totalEnergy = MathHelper::randF(m_EmitterSettings.minEnergy, m_EmitterSettings.maxEnergy);
	p.currentEnergy = p.totalEnergy;

	XMVECTOR unitVector = { 1,0,0 };
	float randomPitch = MathHelper::randF(-XM_PI, XM_PI);
	float randomYaw = MathHelper::randF(-XM_PI, XM_PI);
	float randomRoll = MathHelper::randF(-XM_PI, XM_PI);
	XMMATRIX randomRotMat = DirectX::XMMatrixRotationRollPitchYaw(randomPitch, randomYaw, randomRoll);

	XMVECTOR randomRotVec = XMVector3TransformNormal(unitVector, randomRotMat);
	float randomDistance = MathHelper::randF(m_EmitterSettings.minEmitterRadius, m_EmitterSettings.maxEmitterRadius);
	XMStoreFloat3(&p.vertexInfo.Position, randomRotVec * randomDistance + XMLoadFloat3(&GetTransform()->GetWorldPosition()));

	p.initialSize = MathHelper::randF(m_EmitterSettings.minSize, m_EmitterSettings.maxSize);
	p.vertexInfo.Size = p.initialSize;
	p.sizeChange = p.initialSize * MathHelper::randF(m_EmitterSettings.minScale, m_EmitterSettings.maxScale) - p.initialSize;

	p.vertexInfo.Rotation = MathHelper::randF(-XM_PI, XM_PI);

	p.vertexInfo.Color = m_EmitterSettings.color;
}

void ParticleEmitterComponent::PostDraw(const SceneContext& sceneContext)
{
	m_pParticleMaterial->SetVariable_Matrix(L"gWorldViewProj", sceneContext.pCamera->GetViewProjection());
	m_pParticleMaterial->SetVariable_Matrix(L"gViewInverse", sceneContext.pCamera->GetViewInverse());
	m_pParticleMaterial->SetVariable_Texture(L"gParticleTexture", m_pParticleTexture);

	sceneContext.d3dContext.pDeviceContext->IASetInputLayout(m_pParticleMaterial->GetTechniqueContext().pInputLayout);
	sceneContext.d3dContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT size = sizeof(VertexParticle);
	UINT offset = 0;
	sceneContext.d3dContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &size, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pParticleMaterial->GetTechniqueContext().pTechnique->GetDesc(&techDesc);

	for (size_t i = 0; i < techDesc.Passes; i++)
	{
		m_pParticleMaterial->GetTechniqueContext().pTechnique->GetPassByIndex((uint32_t)i)->Apply(0, sceneContext.d3dContext.pDeviceContext);
		sceneContext.d3dContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}

void ParticleEmitterComponent::DrawImGui()
{
	if (ImGui::CollapsingHeader("Particle System"))
	{
		ImGui::SliderUInt("Count", &m_ParticleCount, 0, m_MaxParticles);
		ImGui::InputFloatRange("Energy Bounds", &m_EmitterSettings.minEnergy, &m_EmitterSettings.maxEnergy);
		ImGui::InputFloatRange("Size Bounds", &m_EmitterSettings.minSize, &m_EmitterSettings.maxSize);
		ImGui::InputFloatRange("Scale Bounds", &m_EmitterSettings.minScale, &m_EmitterSettings.maxScale);
		ImGui::InputFloatRange("Radius Bounds", &m_EmitterSettings.minEmitterRadius, &m_EmitterSettings.maxEmitterRadius);
		ImGui::InputFloat3("Velocity", &m_EmitterSettings.velocity.x);
		ImGui::ColorEdit4("Color", &m_EmitterSettings.color.x, ImGuiColorEditFlags_NoInputs);
	}
}