#pragma once
#include <vector>
#include <unordered_map>
#include <Math/MathStd.hpp>

namespace d3d {

using namespace Math;

struct SkinnedVertex {
	float3  position;
	float2  texcoord;
	float3  normal;
	float3  tangent;
	float3  boneWeights;
	uint8_t boneIndices[4];
};

struct Keyframe {
	Keyframe();
public:
	float  timePoint;
	float3 translation;
	float3 scale;
	float4 rotationQuat;
};

struct BoneAnimation {
	float getStartTime() const;
	float getEndTime() const;
	float4x4 interpolate(float timePoint) const;
	std::vector<Keyframe> keyframes;
};

struct AnimationClip {
	float getClipStartTime() const;
	float getClipEndTime() const;
	std::vector<float4x4> interpolate(float timePoint) const;
	std::vector<BoneAnimation> boneAnimations;
};

class SkinnedData {
public:
	size_t getBoneCount() const;
	float getClipStartTime(const std::string &clipName) const;
	float getClipEndTime(const std::string &clipName) const;
	std::vector<float4x4> getFinalTransforms(const std::string &clipName, float timePoint) const;
	void setBoneHierarchy(const std::vector<size_t> &boneHierarchy);
	void setBoneOffsets(const std::vector<float4x4> &boneOffsets);
	void setAnimations(const std::unordered_map<std::string, AnimationClip> &animations);
	void setBoneHierarchy(std::vector<size_t> &&boneHierarchy);
	void setBoneOffsets(std::vector<float4x4> &&boneOffsets);
	void setAnimations(std::unordered_map<std::string, AnimationClip> &&animations);
private:
	std::vector<size_t> _boneHierarchy;		// 每块骨骼的父节点索引
	std::vector<float4x4> _boneOffsets;
	std::unordered_map<std::string, AnimationClip> _animations;
};

}
