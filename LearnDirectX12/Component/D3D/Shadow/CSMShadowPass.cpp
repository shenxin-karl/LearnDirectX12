#include "CSMShadowPass.h"

#include <iostream>

#include "D3D/Tool/Camera.h"
#include "Dx12lib/Texture/DepthStencilTexture.h"
#include "GameTimer/GameTimer.h"
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
		
	pDirectCtx->setViewport(*customViewport);
	pDirectCtx->setScissorRect(*customScissorRect);

	for (size_t i = 0; i < _numCascaded; ++i) {
		const auto &dsv = _pShadowMapArray->getPlaneDSV(i);
		pDirectCtx->setRenderTarget(dsv);
		auto &boundingBox = _subFrustumItems[i].boundingBox;
		auto iter = _subPasses.begin();
		while (iter != _subPasses.end()) {
			if (!(*iter)->valid()) {
				iter = _subPasses.erase(iter);
				continue;
			}

			auto &pSubPass = *iter;
			if (pSubPass->getJobCount() == 0) {
				++iter;
				continue;
			}

			std::vector<rgph::Job> jobs;
			jobs.reserve(pSubPass->getJobCount());
			for (auto &job : pSubPass->getJobs()) {
				if (boundingBox.contains(job.pGeometry->getWorldAABB()) != DX::ContainmentType::DISJOINT)
					jobs.push_back(job);
			}

			pSubPass->bind(*pDirectCtx);
			auto passCBufferShaderRegister = pSubPass->getPassCBufferShaderRegister();
			auto pPassCb = _subFrustumPassCBuffers[i];
			if (passCBufferShaderRegister.slot && !passCBufferShaderRegister.slot.isSampler())
				pDirectCtx->setConstantBuffer(passCBufferShaderRegister, pPassCb);

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

void CSMShadowPass::setZMulti(float zMulti) {
	_zMulti = zMulti;
}

auto CSMShadowPass::getShadowMapArray() const -> std::shared_ptr<dx12lib::IDepthStencil2DArray> {
	return _pShadowMapArray;
}

auto CSMShadowPass::getShadowTypeCBuffer() const -> FRConstantBufferPtr<CBShadowType> {
	return _pLightSpaceMatrix;
}

auto CSMShadowPass::getShadowMapFormat() const -> DXGI_FORMAT {
	return _shadowMapFormat;
}

void CSMShadowPass::finalize(dx12lib::DirectContextProxy pDirectCtx) {
	assert(!_finalized);
	assert(_numCascaded < kMaxNumCascaded);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = _shadowMapFormat;
	clearValue.DepthStencil.Depth = 1.f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_VIEWPORT viewport = {
		0.f, 0.f,
		static_cast<float>(_shadowMapSize), static_cast<float>(_shadowMapSize),
		0.f, 1.f
	};

	D3D12_RECT rect = {
		0, 0, static_cast<LONG>(_shadowMapSize), static_cast<LONG>(_shadowMapSize)
	};
	customScissorRect = rect;
	customViewport = viewport;

	_pShadowMapArray = pDirectCtx->createDepthStencil2DArray(_shadowMapSize, _shadowMapSize, _numCascaded, &clearValue);
	_pLightSpaceMatrix = pDirectCtx->createFRConstantBuffer<CBShadowType>();
	_pLightSpaceMatrix->setResourceName("LightSpaceMatrix");

	for (size_t i = 0; i < _numCascaded; ++i) {
		auto pPassCb = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
		pPassCb->setResourceName(std::format("SubFrustumPassCBuffer{}", i));
		_subFrustumPassCBuffers.push_back(pPassCb);
	}

	_finalized = true;
}

static BoundingBox calcLightFrustum(const CameraBase *pCameraBase, Vector3 lightDir) {
	BoundingFrustum frustum = pCameraBase->getViewSpaceFrustum();
	Vector3 center(Vector3::identity());
	Vector3 vMin(std::numeric_limits<float>::max());
	Vector3 vMax(std::numeric_limits<float>::min());
	for (auto &c : frustum.getCorners()) {
		Vector3 p(c);
		center += p;
		vMin = min(vMin, p);
		vMax = max(vMax, p);
	}
	center /= 8;

	BoundingBox boundingBox(vMin, vMax);
	Matrix4 lightView = DX::XMMatrixLookAtLH(
		center,
		center + lightDir,
		Vector3(0.f, 1.f, 0.f)
	);
	return boundingBox.transform(lightView);
}

BoundingBox CSMShadowPass::update(const CameraBase *pCameraBase, std::shared_ptr<com::GameTimer> pGameTimer, Vector3 lightDir) {
	_lightDir = lightDir.xyz;

	float zNear = pCameraBase->_nearClip;
	float zFar = pCameraBase->_farClip;
	float ratio = zFar / zNear;
	_subFrustumItems.resize(_numCascaded);
	for (size_t i = 1; i < _numCascaded; ++i) {
		float si = static_cast<float>(i) / static_cast<float>(_numCascaded);
		float z0 = (zNear * pow(ratio, si));
		float z1 = (zNear + (zFar - zNear) * si);
		float tNear = (1.f - _lambda) * z0 + _lambda * z1;
		float tFar = tNear * 1.005f;
		_subFrustumItems[i].zNear = tNear;
		_subFrustumItems[i-1].zFar = tFar;
	}
	_subFrustumItems[0].zNear = zNear;
	_subFrustumItems[_numCascaded-1].zFar = zFar;

	float fov = DX::XMConvertToRadians(pCameraBase->getFov());
	float aspect = pCameraBase->getAspect();
	Matrix4 cameraInvView = inverse(static_cast<Matrix4>(pCameraBase->getView()));
	float2 renderTargetSize(_shadowMapSize);
	float2 invRenderTargetSize(1.f / static_cast<float>(_shadowMapSize), 1.f / static_cast<float>(_shadowMapSize));

	auto pLightSpaceMatrixVisitor = _pLightSpaceMatrix->visit();
	std::memset(pLightSpaceMatrixVisitor.ptr(), 0, sizeof(*pLightSpaceMatrixVisitor));

	float invShadowMapSize = 1.f / static_cast<float>(_shadowMapSize);
	float invZMulti = 1.f / _zMulti;
	float zMulti = _zMulti;

	Matrix4 worldToLightSpace = DX::XMMatrixLookToLH(
		Vector3::zero(), 
		lightDir, 
		Vector3(0.f, 1.f, 0.f)
	);
	Matrix4 lightToWorldSpace = inverse(worldToLightSpace);

	for (size_t i = 0; i < _numCascaded; ++i) {
		FrustumItem &item = _subFrustumItems[i];
		Matrix4 cameraSubProj = DX::XMMatrixPerspectiveFovLH(fov, aspect, item.zNear, item.zFar);
		BoundingFrustum cameraSubViewSpaceFrustum(cameraSubProj);
		cameraSubViewSpaceFrustum = cameraSubViewSpaceFrustum.transform(cameraInvView);
		Vector3 center = Vector3::zero();
		auto corners = cameraSubViewSpaceFrustum.getCorners();
		for (auto &c : corners)
			center += Vector3(c);

		///     Near    Far
		///    0----1  4----5
		///    |    |  |    |
		///    |    |  |    |
		///    3----2  7----6
		float dis1 = length(Vector3(corners[7]) - Vector3(corners[5]));
		float dis2 = length(Vector3(corners[7]) - Vector3(corners[0]));
		float maxDis = std::max(dis1, dis2);
		float disPerPix = maxDis / static_cast<float>(_shadowMapSize);
		center /= 8.f;

		center = worldToLightSpace * Vector4(center, 1.f);
		center = floor(center / disPerPix) * disPerPix;
		center = lightToWorldSpace * Vector4(center, 1);

		Matrix4 lightView = DX::XMMatrixLookAtLH(
			center,
			center + lightDir,
			Vector3(0.f, 1.f, 0.f)
		);

		float orthoNear = std::numeric_limits<float>::max();
		float orthoFar = std::numeric_limits<float>::min();
		BoundingFrustum lightSpaceFrustum = cameraSubViewSpaceFrustum.transform(lightView);
		for (auto &c : lightSpaceFrustum.getCorners()) {
			orthoNear = std::min(orthoNear, c.z);
			orthoFar = std::max(orthoFar, c.z);
		}

		orthoNear *= (orthoNear > 0.f) ? invZMulti : zMulti;
		orthoFar  = std::max(+zFar * 2.f, orthoFar);
		float extentDis = maxDis + static_cast<float>(_pcfKernelSize) * invShadowMapSize * 2.f;
		Matrix4 lightProj = DX::XMMatrixOrthographicLH(
			extentDis, extentDis,
			orthoNear, orthoFar
		);

		Matrix4 invView = inverse(lightView);
		Vector3 bMin(-extentDis * 0.5f, -extentDis * 0.5f, orthoNear);
		Vector3 bMax(+extentDis * 0.5f, extentDis * 0.5f, orthoFar);
		BoundingBox boundingBox(bMin, bMax);
		item.boundingBox = boundingBox.transform(invView);

		Matrix4 lightViewProj = lightProj * lightView;
		Matrix4 scale = Matrix4::makeScale(0.5f, -0.5f, 1.f);
		Matrix4 translation = Matrix4::makeTranslation(0.5f, 0.5f, 0.f);
		Matrix4 ndcToTexcoord = translation * scale;
		Matrix4 worldToShadowTexcoord = ndcToTexcoord * lightViewProj;

		auto pShadowPassCb = _subFrustumPassCBuffers[i];
		auto cbVisitor = pShadowPassCb->visit();
		std::memset(cbVisitor.ptr(), 0, sizeof(*cbVisitor));
		cbVisitor->view = float4x4(lightView);
		cbVisitor->invView = float4x4(invView);
		cbVisitor->proj = float4x4(lightProj);
		cbVisitor->invProj = float4x4(inverse(lightProj));
		cbVisitor->viewProj = float4x4(lightViewProj);
		cbVisitor->invViewProj = float4x4(inverse(lightViewProj));
		cbVisitor->eyePos = center.xyz;
		cbVisitor->renderTargetSize = renderTargetSize;
		cbVisitor->invRenderTargetSize = invRenderTargetSize;
		cbVisitor->nearZ = orthoNear;
		cbVisitor->farZ = orthoFar;
		cbVisitor->totalTime = pGameTimer->getTotalTime();
		cbVisitor->deltaTime = pGameTimer->getDeltaTime();
		pLightSpaceMatrixVisitor->worldToShadowMatrix[i] = float4x4(worldToShadowTexcoord);
	}

	return calcLightFrustum(pCameraBase, lightDir);
}

}
