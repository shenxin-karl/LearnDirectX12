#include "CSMShadowPass.h"
#include "D3D/Tool/Camera.h"
#include "Dx12lib/Texture/DepthStencilTexture.h"
#include "RenderGraph/Pass/SubPass.h"

namespace d3d {

using namespace Math;

ClearCSMShadowMapPass::ClearCSMShadowMapPass(const std::string &passName)
: GraphicsPass(passName, false, false)
, pShadowMapArray(this, "ShadowMapArray")
{
}

void ClearCSMShadowMapPass::execute(dx12lib::DirectContextProxy pDirectCtx) {
	size_t numArray = pShadowMapArray->getPlaneSlice();
	auto pClearValue = pShadowMapArray->getClearValue();
	for (size_t i = 0; i < numArray; ++i) {
		pDirectCtx->clearDepth(
			pShadowMapArray->getPlaneDSV(i),
			pClearValue.DepthStencil.Depth
		);
	}
}

CSMShadowPass::CSMShadowPass(const std::string &name)
: RenderQueuePass(name, false, false)  
, pShadowMapArray(this, "ShadowMapArray")
{
}

void CSMShadowPass::execute(dx12lib::DirectContextProxy pDirectCtx) {
	assert(_finalized);
	for (size_t i = 0; i < _numCascaded; ++i) {
		const auto &dsv = _pShadowMapArray->getPlaneDSV(i);
		pDirectCtx->setRenderTarget(dsv);
		BoundingFrustum frustum(static_cast<Matrix4>(_subFrustumViewProj[i]));
		auto iter = _subPasses.begin();
		while (iter != _subPasses.end()) {
			if (!(*iter)->valid()) {
				iter = _subPasses.erase(iter);
				continue;
			}

			auto &pSubPass = *iter;
			std::vector<rgph::Job> jobs;
			jobs.reserve(pSubPass->getJobCount());

			for (auto &job : pSubPass->getJobs()) {
				if (frustum.contains(job.pGeometry->getWorldAABB()) != DX::ContainmentType::DISJOINT)
					jobs.push_back(job);
			}

			pSubPass->bind(*pDirectCtx);
			auto passCBufferShaderRegister = pSubPass->getPassCBufferShaderRegister();
			if (_pPassCBuffer != nullptr && passCBufferShaderRegister.slot && !passCBufferShaderRegister.slot.isSampler())
				pDirectCtx->setConstantBuffer(passCBufferShaderRegister, _pPassCBuffer);

			pSubPass->execute(*pDirectCtx, jobs);
			++iter;
		}
	}
}

void CSMShadowPass::setNumCascaded(size_t n) {
	_numCascaded = n;
}

void CSMShadowPass::setSplitLambda(float lambda) {
	_lambda = lambda;
}

void CSMShadowPass::setLightDistance(float distance) {
	_lightDistance = distance;
}

auto CSMShadowPass::getShadowMapArray() -> std::shared_ptr<dx12lib::IDepthStencil2DArray> {
	return pShadowMapArray;
}

void CSMShadowPass::finalize(dx12lib::DirectContextProxy pDirectCtx) {
	assert(!_finalized);
	assert(_numCascaded < kMaxNumCascaded);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D16_UNORM;
	clearValue.DepthStencil.Depth = 1.f;
	clearValue.DepthStencil.Stencil = 0;
	_pShadowMapArray = pDirectCtx->createDepthStencil2DArray(_shadowMapSize, _shadowMapSize, _numCascaded, &clearValue);

	for (size_t i = 0; i < _numCascaded; ++i)
		_subFrustumPassCBuffers.push_back(pDirectCtx->createFRConstantBuffer<d3d::CBPassType>());

	_finalized = true;
}

BoundingFrustum CSMShadowPass::update(const CameraBase *pCameraBase, std::shared_ptr<com::GameTimer> pGameTimer, Vector3 lightDir) {
	BoundingFrustum cameraViewSpaceFrustum = pCameraBase->getViewSpaceFrustum();
	return cameraViewSpaceFrustum;
}


static BoundingSphere calcSubFrustumBoundBox(const BoundingFrustum &subFrustum, const Vector3 &lightDir) {
	// 只有旋转没有平移的矩阵
	Matrix4 lightView = DX::XMMatrixLookAtLH(
		Vector3(0.f), 
		lightDir, 
		Vector3(0.f, 1.f, 0.f)
	);

	Vector3 box[2] = { Vector3(std::numeric_limits<float>::max()), Vector3(std::numeric_limits<float>::min()) };
	for (auto &c : subFrustum.getCorners()) {
		Vector3 p = Vector3(lightView * Vector4(c, 1.f));
		box[0] = min(p, box[0]);
		box[1] = max(p, box[1]);
	}

	return { BoundingBox(box[0], box[1]) };
}

Vector3 CSMShadowPass::calcLightCenter(const BoundingFrustum &cameraSubFrustum) {
	Vector3 center(0.f);

	// 算出视锥体的中心点
	auto corners = cameraSubFrustum.getCorners();
	for (auto &c : corners)
		center += Vector3(c);

	center /= 8.f;
	return center;
}

struct FrustumItem {
	float zNear;
	float zFar;
};
void CSMShadowPass::updateSubFrustumViewProj(const CameraBase *pCameraBase, Vector3 lightDir) {
	_subFrustumViewProj.resize(_numCascaded);
	std::vector<FrustumItem> split(_numCascaded);

	float zNear = pCameraBase->_nearClip;
	float zFar = pCameraBase->_farClip;
	float ratio = zFar / zNear;
	for (size_t i = 1; i < _numCascaded; ++i) {
		float si = static_cast<float>(i) / static_cast<float>(_numCascaded);
		float z0 = (zNear * pow(ratio, si));
		float z1 = (zNear + (zFar - zNear) * si);
		float tNear = (1.f - _lambda) * z0 + _lambda * z1;
		float tFar = tNear * 1.005f;
		split[i].zNear = tNear;
		split[i-1].zFar = tFar;
	}
	split[0].zNear = zNear;
	split[_numCascaded-1].zFar = zFar;

	float fov = pCameraBase->getFov();
	float aspect = pCameraBase->getAspect();
	for (size_t i = 0; i < _numCascaded; ++i) {
		FrustumItem &item = split[i];
		Matrix4 cameraSubProj = DX::XMMatrixPerspectiveFovLH(fov, aspect, item.zNear, item.zFar);
		BoundingFrustum cameraSubFrustum(cameraSubProj);
		Vector3 center = calcLightCenter(cameraSubFrustum);
		Matrix4 lightView = DX::XMMatrixLookAtLH(
			center, 
			center + lightDir, 
			Vector3(0.f, 1.f, 0.f)
		);

		Vector3 boxMin;
		Vector3 boxMax;
		BoundingSphere boundBox = calcSubFrustumBoundBox(cameraSubFrustum, lightDir);
		Matrix4 lightProj = DX::XMMatrixPerspectiveOffCenterLH(
			boxMin.x, boxMax.x,
			boxMin.y, boxMax.y,
			boxMin.z, boxMax.z
		);

		Matrix4 lightViewProj = lightProj * lightView;
		_subFrustumViewProj[i] = static_cast<float4x4>(lightViewProj);
	}
}

}
