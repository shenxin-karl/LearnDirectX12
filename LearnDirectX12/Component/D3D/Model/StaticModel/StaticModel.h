#pragma once
#include <D3D/d3dutil.h>
#include <D3D/Model/ModelInterface.hpp>
#include <D3D/TextureManager/TextureManager.h>
#include <D3D/Shader/ShaderCommon.h>
#include <D3D/AssimpLoader/AssimpLoader.h>

namespace d3d {

class StaticSubModel : public ISubModel {
public:
	StaticSubModel() = default;
	void initAsALMesh(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader &loader, const AssimpLoader::ALMesh &alMesh);
	const d3d::Material &getMaterial() const override;
	std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer() const override;
	std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer() const override;
	const std::vector<D3D12_INPUT_ELEMENT_DESC> &getInputLayout() const override;
private:
	d3d::Material _material;
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer> _pIndexBuffer;
};

class StaticModel : public IModel {
public:
	StaticModel(const std::string &modelName);
	void initAsAssimpLoader(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader &loader);
	const std::string &getModelName() const override;
	size_t getSubModelCount() const override;
	std::shared_ptr<ISubModel> getSubModel(size_t subId) const override;
private:
	std::string _modelName;
	std::vector<std::shared_ptr<StaticSubModel>> _subModels;
};

}