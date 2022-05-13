#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter) :
	m_pMeshFilter{ pMeshFilter }
{
	SetAnimation(0);
}

void ModelAnimator::Update(const SceneContext& sceneContext)
{
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		auto passedTicks = sceneContext.pGameTime->GetElapsed() * m_AnimationSpeed * m_CurrentClip.ticksPerSecond;
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, m_CurrentClip.duration);
		//2. 
		//IF m_Reversed is true
		if (m_Reversed)
		{
			//	Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
		}
		//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
		if (m_TickCount < 0)
		{
			m_TickCount += m_CurrentClip.duration;
		}
		//ELSE
		else
		{
			//	Add passedTicks to m_TickCount
			m_TickCount += passedTicks;
		}
		//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
		if (m_TickCount > m_CurrentClip.duration)
		{
			m_TickCount -= m_CurrentClip.duration;
		}
		//3.
		//Find the enclosing keys
		AnimationKey keyA, keyB;
		//Iterate all the keys of the clip and find the following keys:
		for (unsigned int i{}; i < m_CurrentClip.keys.size(); ++i)
		{
			//keyA > Closest Key with Tick before/smaller than m_TickCount
			//keyB > Closest Key with Tick after/bigger than m_TickCount
			if (m_TickCount > m_CurrentClip.keys[i].tick)
			{
				keyA = m_CurrentClip.keys[i];
				if (i + 1 == m_CurrentClip.keys.size())
				{
					keyB = m_CurrentClip.keys[i];

				}
				else
				{
					keyB = m_CurrentClip.keys[i + 1];
				}
			}
		}
		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		float tickA = keyA.tick;
		float tickB = keyB.tick;

		float blendFactor = (m_TickCount - tickA) / (tickB - tickA);
		//Clear the m_Transforms vector
		m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (unsigned int i = 0; i < keyA.boneTransforms.size(); ++i)
		{
			//	Retrieve the transform from keyA (transformA)
			auto transformA = keyA.boneTransforms[i];
			// 	Retrieve the transform from keyB (transformB)
			auto transformB = keyB.boneTransforms[i];
				//	Decompose both transforms
			XMMATRIX matrixA = XMLoadFloat4x4(&transformA);
			XMMATRIX matrixB = XMLoadFloat4x4(&transformB);

			XMVECTOR scaleA, scaleB, quatRotA, quatRotB, transA, transB;
			XMMatrixDecompose(&scaleA, &quatRotA, &transA, matrixA);
			XMMatrixDecompose(&scaleB, &quatRotB, &transB, matrixB);
				//	Lerp between all the transformations (Position, Scale, Rotation)
			auto lerpTransform = XMVectorLerp(transA, transB, blendFactor);
			auto lerpScale = XMVectorLerp(scaleA, scaleB,blendFactor);
			auto lerpRot = XMQuaternionSlerp(quatRotA, quatRotB, blendFactor);
				//	Compose a transformation matrix with the lerp-results
			XMMATRIX transMat = XMMatrixScalingFromVector(lerpScale) *
				XMMatrixRotationQuaternion(lerpRot) *
				XMMatrixTranslationFromVector(lerpTransform);

				//	Add the resulting matrix to the m_Transforms vector
			XMFLOAT4X4 result;
			XMStoreFloat4x4(&result, transMat);
			m_Transforms.push_back(result);
		}
	}
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	//Set m_ClipSet to false

	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	for (unsigned int i = 0; i < m_pMeshFilter->m_AnimationClips.size(); ++i)
	{
		//If found,
		if (m_pMeshFilter->m_AnimationClips[i].name == clipName)
		{
			//	Call SetAnimation(Animation Clip) with the found clip
			SetAnimation(m_pMeshFilter->m_AnimationClips[i]);
			return;
		}
	}
	Reset();
	Logger::LogWarning(L"Warning: clipName not found");

	//Else
	//	Call Reset
	//	Log a warning with an appropriate message
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{

	m_ClipSet = false;
	//Set m_ClipSet to false
//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if (!(clipNumber < m_pMeshFilter->m_AnimationClips.size()))
	{
		Reset();
		Logger::LogWarning(L"Warning: invalid clipNumber");
		return;
	}
	else
	{
		AnimationClip animationClip = m_pMeshFilter->m_AnimationClips[clipNumber];
		SetAnimation(animationClip);
	}

	//If not,
		//	Call Reset
		//	Log a warning with an appropriate message
		//	return
	//else
		//	Retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
		//	Call SetAnimation(AnimationClip clip)
}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	m_ClipSet = true;
	//Set m_ClipSet to true
	//Set m_CurrentClip
	m_CurrentClip = clip;
	Reset(false);
	//Call Reset(false)
}

void ModelAnimator::Reset(bool pause)
{
	//If pause is true, set m_IsPlaying to false
	if (pause)
	{
		m_IsPlaying = false;
	}
	m_TickCount = 0;
	m_AnimationSpeed = 1.f;
	//Set m_TickCount to zero
	//Set m_AnimationSpeed to 1.0f

	//If m_ClipSet is true
	if (m_ClipSet)
	{
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		auto boneTransform = m_CurrentClip.keys[0].boneTransforms;
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(boneTransform.begin(), boneTransform.end());
	}
	else
	{
		XMFLOAT4X4 identityMatrix = { 1,0,0,0,
									0,1,0,0,
									0,0,1,0,
									0,0,0,1 };

		m_Transforms.assign(m_Transforms.size(), identityMatrix);
	}
	//Else
	//	Create an IdentityMatrix 
	//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
}
