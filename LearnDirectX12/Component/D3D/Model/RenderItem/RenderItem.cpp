#include "RenderItem.h"
#include "D3D/Model/Material/Material.h"

namespace d3d {

RenderItem::RenderItem(dx12lib::IDirectContext &directCtx, std::shared_ptr<ALMesh> pALMesh) : _pMesh(pALMesh) {
	const auto &indices = pALMesh->getIndices();
	if (indices.size() < 3)
		return;

	std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer;
	if (pALMesh->getPositions().size() > std::numeric_limits<uint16_t>::max()) {
		std::string key = pALMesh->getMeshName() + "_IndexBuffer_uint32";
		pIndexBuffer = MeshManager::instance()->getIndexBuffer(key);
		if (pIndexBuffer == nullptr) {
			pIndexBuffer = directCtx.createIndexBuffer(
				indices.data(), 
				indices.size(), 
				DXGI_FORMAT_R32_UINT
			);
			MeshManager::instance()->setIndexBuffer(key, pIndexBuffer);
		}
	} else {
		std::string key = pALMesh->getMeshName() + "_IndexBuffer_uint16";
		pIndexBuffer = MeshManager::instance()->getIndexBuffer(key);
		if (pIndexBuffer == nullptr) {
			std::vector<uint16_t> newIndices;
			newIndices.resize(indices.size());
			for (size_t i = 0; i < indices.size()-2; i += 3) {
				newIndices[i+0] = static_cast<uint16_t>(indices[i+0]);
				newIndices[i+1] = static_cast<uint16_t>(indices[i+1]);
				newIndices[i+2] = static_cast<uint16_t>(indices[i+2]);
			}

			pIndexBuffer = directCtx.createIndexBuffer(
				newIndices.data(), 
				newIndices.size(), 
				DXGI_FORMAT_R16_UINT
			);
			MeshManager::instance()->setIndexBuffer(key, pIndexBuffer);
		}
	}

	setIndexBuffer(pIndexBuffer);
}

std::shared_ptr<Material> RenderItem::getMaterial() const {
	return _pMaterial;
}

std::shared_ptr<ALMesh> RenderItem::getMesh() const {
	return _pMesh;
}

void RenderItem::setMaterial(std::shared_ptr<Material> pMaterial) {
	_pMaterial = pMaterial;
}

void RenderItem::rebuildTechnique() {
	clearTechnique();
	if (_pMaterial == nullptr)
		return;

	for (size_t i = 0; i < _pMaterial->getNumTechnique(); ++i)
		addTechnique(_pMaterial->getTechnique(i));
}


bool RenderItem::buildVertexDataInput(dx12lib::IDirectContext &directCtx, const VertexDataSemantic &semantic) {
	if (getVertexBuffer(semantic.slot) != nullptr)
		return false;

	std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer = nullptr;
	if (semantic == PositionSemantic)
		pVertexBuffer = buildVertexDataInputImpl(directCtx, semantic, _pMesh->getPositions());
	else if (semantic == NormalSemantic)
		pVertexBuffer = buildVertexDataInputImpl(directCtx, semantic, _pMesh->getNormals());
	else if (semantic == TangentSemantic)
		pVertexBuffer = buildVertexDataInputImpl(directCtx, semantic, _pMesh->getTangents());
	else if (semantic == Texcoord0Semantic)
		pVertexBuffer = buildVertexDataInputImpl(directCtx, semantic, _pMesh->getTexcoord0());
	else if (semantic == Texcoord1Semantic)
		pVertexBuffer = buildVertexDataInputImpl(directCtx, semantic, _pMesh->getTexcoord1());

	assert(pVertexBuffer != nullptr);
	setVertexBuffer(pVertexBuffer, semantic.slot);
	return true;
}

}
