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

		BoundingFrustum frustum(static_cast<Matrix4>(_subFrustumItems[i].matProj));
		frustum = frustum.transform(inverse(static_cast<Matrix4>(_subFrustumItems[i].matView)));
		Vector3 vMin(std::numeric_limits<float>::max());
		Vector3 vMax(std::numeric_limits<float>::min());
		for (auto &c : frustum.getCorners()) {
			Vector3 p(c);
			vMin = min(vMin, p);
			vMax = max(vMax, p);
		}

		BoundingBox boundingBox(vMin, vMax);


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

void CSMShadowPass::setLightDistance(float distance) {
	_lightDistance = distance;
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

	for (size_t i = 0; i < _numCascaded; ++i)
		_subFrustumPassCBuffers.push_back(pDirectCtx->createFRConstantBuffer<d3d::CBPassType>());

	_finalized = true;
}

std::array<float3, 8> getFrustumCornersWorldSpace(const Matrix4 &projview) {
	size_t i = 0;
	std::array<float3, 8> frustumCorners;
	Matrix4 invViewProj = inverse(projview);

	for (unsigned int x = 0; x < 2; ++x) {
		for (unsigned int y = 0; y < 2; ++y) {
			for (unsigned int z = 0; z < 2; ++z) {
				const Vector4 pt = invViewProj * Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners[i] = (pt / pt.w).xyz;
				++i;
			}
		}
	}
	return frustumCorners;
}

static BoundingBox calcSubFrustumBoundBox(const BoundingFrustum &subFrustum, const Vector3 &lightDir) {
	//todo: 避免转动相机时出现抖动, 使用包围球
	Vector3 box[2] = { Vector3(std::numeric_limits<float>::max()), Vector3(std::numeric_limits<float>::min()) };
	for (auto &c : subFrustum.getCorners()) {
		// viewSpacePos = proj * view * p
		// viewSpacePos = inverse(view) * proj * p;
		//todo: 求出世界空间的包围球, 因为相机没有 M 变换, 所以 VP 就是世界空间
		Vector3 p = Vector4(c, 1.f);
		box[0] = min(p, box[0]);
		box[1] = max(p, box[1]);
	}
	BoundingBox boundingBox(box[0], box[1]);
	return boundingBox;
}

Vector3 CSMShadowPass::calcLightCenter(const BoundingSphere &boundingSphere, const Math::Vector3 &lightDir) const {
	Vector3 center = Vector3(boundingSphere.getCenter());
	Matrix4 matLightSpace = DX::XMMatrixLookAtLH(
		center,
		center + lightDir,
		Vector3(0.f, 1.f, 0.f)
	);

	//todo: 避免阴影移动时抖动, 做个对齐处理, 必须对齐 boundingBox.size / shadowMap.size
	Matrix4 matInvLightSpace = inverse(matLightSpace);
	center = Vector3(matLightSpace * Vector4(center, 1.f));
	float aligned = boundingSphere.getRadius() / static_cast<float>(_shadowMapSize);
	for (size_t i = 0; i < 3; ++i)
		center[i] = std::floor(center[i] / aligned) * aligned;

	center = matInvLightSpace * Vector4(center, 1.f);
	return center;
}

BoundingSphere calcLightBoundingSphere(const BoundingFrustum &cameraSubFrustum, const Matrix4 &lightView) {
	Vector3 vMin(std::numeric_limits<float>::max());
	Vector3 vMax(std::numeric_limits<float>::min());
	for (auto &c : cameraSubFrustum.getCorners()) {
		Vector3 p = lightView * Vector4(c, 1.f);
		vMin = min(vMin, p);
		vMax = max(vMax, p);
	}
	return BoundingBox(vMin, vMax);
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
	_subFrustumItems.resize(_numCascaded);
	float zNear = pCameraBase->_nearClip;
	float zFar = pCameraBase->_farClip;
	float ratio = zFar / zNear;
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
	static int count = 0;
	++count;

	for (size_t i = 0; i < _numCascaded; ++i) {
		FrustumItem &item = _subFrustumItems[i];
		Matrix4 cameraSubProj = DX::XMMatrixPerspectiveFovLH(fov, aspect, item.zNear, item.zFar);
		BoundingFrustum cameraSubFrustum(cameraSubProj);
		cameraSubFrustum = cameraSubFrustum.transform(cameraInvView);
		BoundingBox boundingBox = calcSubFrustumBoundBox(cameraSubFrustum, lightDir);
		Vector3 center = calcLightCenter(boundingBox, lightDir);

		Matrix4 lightView = DX::XMMatrixLookAtLH(
			center,
			center + lightDir,
			Vector3(0.f, 1.f, 0.f)
		);

		BoundingSphere lightCameraBoundingSphere = calcLightBoundingSphere(cameraSubFrustum, lightView);
		auto &&[vMin, vMax] = BoundingBox(lightCameraBoundingSphere).getMinMax();

		Matrix4 lightProj = DX::XMMatrixOrthographicOffCenterLH(
			vMin.x, vMax.x,
			vMin.y, vMax.y,
			vMin.z, vMax.z
		);

		item.matView = float4x4(lightView);
		item.matProj = float4x4(lightProj);

		Matrix4 lightViewProj = lightProj * lightView;

		auto pShadowPassCb = _subFrustumPassCBuffers[i];
		auto cbVisitor = pShadowPassCb->visit();
		std::memset(cbVisitor.ptr(), 0, sizeof(*cbVisitor));
		cbVisitor->view = float4x4(lightView);
		cbVisitor->invView = float4x4(inverse(lightView));
		cbVisitor->proj = float4x4(lightProj);
		cbVisitor->invProj = float4x4(inverse(lightProj));
		cbVisitor->viewProj = float4x4(lightViewProj);
		cbVisitor->invViewProj = float4x4(inverse(lightViewProj));
		cbVisitor->eyePos = center.xyz;
		cbVisitor->renderTargetSize = renderTargetSize;
		cbVisitor->invRenderTargetSize = invRenderTargetSize;
		cbVisitor->nearZ = vMin.z;
		cbVisitor->farZ = vMax.z;
		cbVisitor->totalTime = pGameTimer->getTotalTime();
		cbVisitor->deltaTime = pGameTimer->getDeltaTime();
		pLightSpaceMatrixVisitor->lightSpaceMatrix[i] = cbVisitor->viewProj;
	}

	return calcLightFrustum(pCameraBase, lightDir);
}

}
