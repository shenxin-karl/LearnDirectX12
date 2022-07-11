#include "StaticModel.h"
#include "dx12lib/Pipeline/PipelineStateObject.h"

namespace d3d {

void StaticSubModel::initAsALMesh(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader &loader, const AssimpLoader::ALMesh &alMesh) {
	aiMaterial *pAiMaterial = alMesh.pAiMaterial;
	if (pAiMaterial == nullptr)
		return;

	auto prepareMaterialTexture = [&](aiTextureType type, bool sRGB) -> std::optional<std::string> {
		if (pAiMaterial->GetTextureCount(type) <= 0)
			return std::nullopt;

		aiString path;
		pAiMaterial->GetTexture(type, 0, &path);
		if (path.data[0] == '*') {						// 如果是 * 开头,表示是内嵌的贴图, 这里拼接上一个贴图名
			int textureIndex = 0;
			sscanf_s(path.data, "*%d", &textureIndex);
			const aiScene *pScene = loader.getScene();
			assert(static_cast<size_t>(textureIndex) < pScene->mNumTextures);
			const aiTexture *pAiTexture = pScene->mTextures[textureIndex];

			std::string textureName = loader.getFileName() + pAiTexture->mFilename.C_Str();;
			if (TextureManager::instance()->exist(textureName))
				return textureName;

			assert(pAiTexture->mHeight != 0 && "Embedded maps can only be compressed textures");
			auto pTexture = pGraphicsCtx->createTextureFromMemory(pAiTexture->achFormatHint,
				pAiTexture->pcData,
				pAiTexture->mWidth,
				sRGB
			);
			TextureManager::instance()->set(textureName, pTexture);
			return textureName;
		}

		std::string textureName = path.C_Str();
		auto pTexture = pGraphicsCtx->createTextureFromFile(std::to_wstring(textureName), sRGB);
		TextureManager::instance()->set(textureName, pTexture);
		return textureName;
	};

	if (auto texName = prepareMaterialTexture(aiTextureType_DIFFUSE, true))
		_material.setalbedoMapName(*texName);
	if (auto texName = prepareMaterialTexture(aiTextureType_METALNESS, false))
		_material.setmetallicMapName(*texName);
	if (auto texName = prepareMaterialTexture(aiTextureType_DIFFUSE_ROUGHNESS, false))
		_material.setroughnessMapName(*texName);
	if (auto texName = prepareMaterialTexture(aiTextureType_NORMALS, false))
		_material.setnormalMapName(*texName);
	if (auto texName = prepareMaterialTexture(aiTextureType_AMBIENT_OCCLUSION, false))
		_material.setaoMapName(*texName);
}

const d3d::Material & StaticSubModel::getMaterial() const {
	return _material;
}

std::shared_ptr<dx12lib::VertexBuffer> StaticSubModel::getVertexBuffer() const {
	return _pVertexBuffer;
}

std::shared_ptr<dx12lib::IndexBuffer> StaticSubModel::getIndexBuffer() const {
	return _pIndexBuffer;
}

const std::vector<D3D12_INPUT_ELEMENT_DESC> & StaticSubModel::getInputLayout() const {
	static std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = {
		dx12lib::VInputLayoutDescHelper(&com::Vertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&com::Vertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&com::Vertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&com::Vertex::tangent, "TANGENT", DXGI_FORMAT_R32G32B32_FLOAT),
	};
	return inputElementDescs;
}


///************************************************************************************************

StaticModel::StaticModel(const std::string &modelName) : _modelName(modelName) {
}

void StaticModel::initAsAssimpLoader(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader &loader) {
	_subModels.clear();
	auto meshs = loader.parseMesh();
	for (AssimpLoader::ALMesh &mesh : meshs) {
		auto pSubMesh = std::make_shared<StaticSubModel>();
		pSubMesh->initAsALMesh(pGraphicsCtx, loader, mesh);
		_subModels.push_back(pSubMesh);
	}
}

const std::string &StaticModel::getModelName() const {
	return _modelName;
}

size_t StaticModel::getSubModelCount() const {
	return _subModels.size();
}

std::shared_ptr<ISubModel> StaticModel::getSubModel(size_t subId) const {
	assert(subId > 0 && subId < getSubModelCount());
	auto &pSubModel = _subModels[subId];
	return std::static_pointer_cast<ISubModel>(pSubModel);
}

}
