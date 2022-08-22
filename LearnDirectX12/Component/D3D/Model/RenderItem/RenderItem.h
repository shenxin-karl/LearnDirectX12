#pragma once
#include <RenderGraph/Drawable/Drawable.h>
#include "D3D/AssimpLoader/ALNode.h"
#include "D3D/Model/RenderItem/VertexDataSemantic.h"
#include "D3D/Model/Mesh/MeshManager.h"

namespace d3d {

struct INode;
class Material;
class RenderItem : public rg::Drawable {
public:
	RenderItem(dx12lib::IDirectContext &directCtx, std::shared_ptr<ALMesh> pALMesh);
	std::shared_ptr<Material> getMaterial() const;
	std::shared_ptr<ALMesh> getMesh() const;
	void setMaterial(std::shared_ptr<Material> pMaterial);
	void rebuildTechnique();
	bool buildVertexDataInput(dx12lib::IDirectContext &directCtx, const VertexDataSemantic &semantic);
private:
	template<typename T>
	std::shared_ptr<dx12lib::VertexBuffer> buildVertexDataInputImpl(dx12lib::IDirectContext &directCtx,
		const VertexDataSemantic &semantic, 
		const std::vector<T> &data)
	{
		assert(!data.empty());
		std::string key = _pMesh->getMeshName() + std::format("_{}", semantic.name);
		auto pVertexBuffer = MeshManager::instance()->getVertexBuffer(key);
		if (pVertexBuffer == nullptr) {
			pVertexBuffer = directCtx.createVertexBuffer(data.data(), data.size(), sizeof(T));
			MeshManager::instance()->setVertexBuffer(key, pVertexBuffer);
		}
		return pVertexBuffer;
	}
private:
	std::shared_ptr<ALMesh> _pMesh;
	std::shared_ptr<Material> _pMaterial;
};

}
