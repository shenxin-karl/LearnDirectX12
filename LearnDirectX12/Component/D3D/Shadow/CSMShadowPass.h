#pragma once
#include <RenderGraph/Pass/RenderQueuePass.h>
#include <Dx12lib/Context/ContextProxy.hpp>
#include "D3D/Shader/ShaderCommon.h"

namespace com {
class GameTimer;
}

namespace d3d {

class CameraBase;
class CSMShadowPass;

class ClearCSMShadowMapPass : public rgph::GraphicsPass {
public:
	ClearCSMShadowMapPass(const std::string &passName);
	void execute(dx12lib::DirectContextProxy pDirectCtx) override; 
	rgph::PassResourcePtr<dx12lib::IDepthStencil2DArray> pShadowMapArray;
};

class CSMShadowPass : public rgph::RenderQueuePass {
public:
	constexpr static size_t kMaxNumCascaded = 7;
	struct CBShadowType {
		Math::float4x4 lightSpaceMatrix[kMaxNumCascaded];
	};
	struct FrustumItem {
		float		   zNear;
		float		   zFar;
		Math::float4x4 matView;
		Math::float4x4 matProj;
	};
public:
	CSMShadowPass(const std::string &name);
	void execute(dx12lib::DirectContextProxy pDirectCtx) override;
	void setNumCascaded(size_t n);
	void setSplitLambda(float lambda);
	void setLightDistance(float distance);
	auto getShadowMapArray() const -> std::shared_ptr<dx12lib::IDepthStencil2DArray>;
	auto getShadowTypeCBuffer() const -> FRConstantBufferPtr<CBShadowType>;
	auto getShadowMapFormat() const -> DXGI_FORMAT;
	void finalize(dx12lib::DirectContextProxy pDirectCtx);
	Math::BoundingBox update(const CameraBase *pCameraBase, std::shared_ptr<com::GameTimer> pGameTimer, Math::Vector3 lightDir);

	rgph::PassResourcePtr<dx12lib::IDepthStencil2DArray> pShadowMapArray;
private:
	Math::Vector3 calcLightCenter(const Math::BoundingSphere &boundingSphere, const Math::Vector3 &lightDir) const;
private:
	bool _finalized = false;
	float _lambda = 0.5f;
	float _lightDistance = 512.f;
	size_t _numCascaded = 4;
	size_t _shadowMapSize = 512;
	DXGI_FORMAT _shadowMapFormat = DXGI_FORMAT_D16_UNORM;
	std::vector<FrustumItem> _subFrustumItems;
	FRConstantBufferPtr<CBShadowType> _pLightSpaceMatrix;
	std::shared_ptr<dx12lib::IDepthStencil2DArray> _pShadowMapArray;
	std::vector<FRConstantBufferPtr<d3d::CBPassType>> _subFrustumPassCBuffers;
};

}
