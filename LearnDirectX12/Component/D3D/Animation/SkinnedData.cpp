#include <D3D/Animation/SkinnedData.h>

namespace d3d {

Keyframe::Keyframe() : timePoint(0.f), scale(1.f) {
}

float BoneAnimation::getStartTime() const {
	return keyframes.front().timePoint;
}

float BoneAnimation::getEndTime() const {
	return keyframes.back().timePoint;
}

float4x4 BoneAnimation::interpolate(float timePoint) const {
	assert(!keyframes.empty());
	Vector4 origin{ 0.f, 0.f, 0.f, 1.f };
	if (timePoint <= getStartTime()) {
		const auto &first = keyframes.front();
		return static_cast<float4x4>(Matrix4(DirectX::XMMatrixAffineTransformation(
			Vector3(first.scale),
			origin,
			Quaternion(first.rotationQuat),
			Vector3(first.translation)
		)));
	}
	if (timePoint >= getEndTime()) {
		const auto &back = keyframes.back();
		return static_cast<float4x4>(Matrix4(DirectX::XMMatrixAffineTransformation(
			Vector3(back.scale),
			origin,
			Quaternion(back.rotationQuat),
			Vector3(back.translation)
		)));
	}
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		bool inside = (timePoint >= keyframes[i].timePoint && timePoint < keyframes[i + 1].timePoint);
		if (!inside)
			continue;

		float t = (timePoint - keyframes[i].timePoint) / (keyframes[i + 1].timePoint - keyframes[i].timePoint);
		Vector3 scale = lerp(Vector3(keyframes[i].scale), Vector3(keyframes[i+1].scale), t);
		Vector3 translation = lerp(Vector3(keyframes[i].translation), Vector3(keyframes[i+1].translation), t);
		Quaternion rotate = slerp(Quaternion(keyframes[i].rotationQuat), Quaternion(keyframes[i+1].rotationQuat), t);
		return static_cast<float4x4>(Matrix4(DirectX::XMMatrixAffineTransformation(
			scale,
			origin,
			rotate,
			translation
		)));
	}
	assert(false);
	return float4x4::identity();
}

float AnimationClip::getClipStartTime() const {
	if (boneAnimations.empty())
		return 0.f;
	return boneAnimations.front().getStartTime();
}

float AnimationClip::getClipEndTime() const {
	if (boneAnimations.empty())
		return 0.f;
	return boneAnimations.back().getEndTime();
}

std::vector<float4x4> AnimationClip::interpolate(float timePoint) const {
	std::vector<float4x4> result;
	result.reserve(boneAnimations.size());
	for (auto &bone : boneAnimations)
		result.push_back(bone.interpolate(timePoint));

	return result;
}

size_t SkinnedData::getBoneCount() const {
	return _boneHierarchy.size();
}

float SkinnedData::getClipStartTime(const std::string &clipName) const {
	if (auto iter = _animations.find(clipName); iter != _animations.end())
		return iter->second.getClipStartTime();

	assert(false);
	return 0.f;
}

float SkinnedData::getClipEndTime(const std::string &clipName) const {
	if (auto iter = _animations.find(clipName); iter != _animations.end())
		return iter->second.getClipStartTime();

	assert(false);
	return 0.f;
}

std::vector<float4x4> SkinnedData::getFinalTransforms(const std::string &clipName, float timePoint) const {
	auto iter = _animations.find(clipName);
	if ( iter == _animations.end()) {
		assert(false);
		return {};
	}

	std::vector<float4x4> boneTransform = iter->second.interpolate(timePoint);
	std::vector<Matrix4> toRootSpaceTransforms(_boneOffsets.size());
	toRootSpaceTransforms[0] = static_cast<Matrix4>(boneTransform[0]);
	for (size_t i = 1; i < boneTransform.size(); ++i) {
		size_t parentIndex = _boneHierarchy[i];
		Matrix4 toParent { boneTransform[i] };
		const Matrix4 &parentToRoot = toRootSpaceTransforms[parentIndex];
		Matrix4 toRoot = parentToRoot * toParent;
		toRootSpaceTransforms[i] = toRoot;
	}

	std::vector<float4x4> result;
	result.reserve(_boneOffsets.size());
	for (size_t i = 0; i < toRootSpaceTransforms.size(); ++i) {
		Matrix4 offset { _boneOffsets[i] };
		const Matrix4 &toRootSpace = toRootSpaceTransforms[i];
		result.push_back(static_cast<float4x4>(toRootSpace * offset));
	}
	return result;
}

void SkinnedData::setBoneHierarchy(const std::vector<size_t> &boneHierarchy) {
	auto tmp = boneHierarchy;
	setBoneHierarchy(std::move(tmp));
}

void SkinnedData::setBoneOffsets(const std::vector<float4x4> &boneOffsets) {
	auto tmp = boneOffsets;
	setBoneOffsets(std::move(tmp));
}

void SkinnedData::setAnimations(const std::unordered_map<std::string, AnimationClip> &animations) {
	auto tmp = animations;
	setAnimations(std::move(tmp));
}

void SkinnedData::setBoneHierarchy(std::vector<size_t> &&boneHierarchy) {
	_boneHierarchy = std::move(boneHierarchy);
}

void SkinnedData::setBoneOffsets(std::vector<float4x4> &&boneOffsets) {
	_boneOffsets = std::move(boneOffsets);
}

void SkinnedData::setAnimations(std::unordered_map<std::string, AnimationClip> &&animations) {
	_animations = std::move(animations);
}


}
