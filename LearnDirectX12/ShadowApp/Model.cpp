#include "Model.h"
#include "D3D/AssimpLoader/AssimpLoader.h"
#include "D3D/Tool/Mesh.h"

Model::Model(dx12lib::DirectContextProxy pDirectCtx, const d3d::AssimpLoader &loader) {
	auto meshs = loader.parseMesh();
	assert(meshs.size() == 1);
	auto pVertexBuffer = pDirectCtx->createVertexBuffer(
		meshs[0].vertices.data(),
		meshs[0].vertices.size(),
		sizeof(com::Vertex)
	);
	auto pIndexBuffer = pDirectCtx->createIndexBuffer(
		meshs[0].indices.data(),
		meshs[0].indices.size(),
		DXGI_FORMAT_R16_UINT
	);
	_pMesh = std::make_shared<d3d::Mesh>(pVertexBuffer, pIndexBuffer);

	aiMaterial *pAiMaterial = meshs[0].pAiMaterial;


}

std::shared_ptr<Model::ModelRenderItem> Model::getModelRenderItem() const {
	return _pModelRenderItem;
}

std::shared_ptr<Model::ShadowRenderItem> Model::getShadowRenderItem() const {
	return _pShadowRenderItem;
}

Model::CBObjectType &Model::getCbObject() {
	return *_pObjectCb->visit();
}

const Model::CBObjectType &Model::getCbObject() const {
	return *_pObjectCb->visit();
}
