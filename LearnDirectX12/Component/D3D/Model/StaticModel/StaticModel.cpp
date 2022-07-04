#include "StaticModel.h"
#include "StaticGeometryInput.h"

namespace d3d {

void StaticSubModel::initAsALMesh(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader &loader, const AssimpLoader::ALMesh &alMesh) {
	_pGeometryInput = std::make_shared<StaticGeometryInput>(pGraphicsCtx, alMesh);
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
			assert(textureIndex < pScene->mNumTextures);
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
		_albedoMapName = *texName;
	if (auto texName = prepareMaterialTexture(aiTextureType_METALNESS, false))
		_metallicMapName = *texName;
	if (auto texName = prepareMaterialTexture(aiTextureType_DIFFUSE_ROUGHNESS, false))
		_roughnessMapName = *texName;
	if (auto texName = prepareMaterialTexture(aiTextureType_NORMALS, false))
		_normalMapName = *texName;
	if (auto texName = prepareMaterialTexture(aiTextureType_AMBIENT_OCCLUSION, false))
		_aoMapName = *texName;
}

MaterialData StaticSubModel::getMaterialData() const {
	return _materialData;
}

std::string StaticSubModel::getAOMapName() const {
	return _aoMapName;
}

std::string StaticSubModel::getAlbedoMapName() const {
	return _albedoMapName;
}

std::string StaticSubModel::getNormalMapName() const {
	return _normalMapName;
}

std::string StaticSubModel::getMetallicMapName() const {
	return _metallicMapName;
}

std::string StaticSubModel::getRoughnessMapName() const {
	return _roughnessMapName;
}

std::shared_ptr<IGeometryInput> StaticSubModel::getGeometryInput() const {
	return _pGeometryInput;
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