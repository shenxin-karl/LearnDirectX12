#include "StaticModel.h"
#include "StaticGeometryInput.h"

namespace d3d {

void StaticSubModel::initAsALMesh(const AssimpLoader &loader, const AssimpLoader::ALMesh &alMesh) {
	_pGeometryInput = std::make_shared<StaticGeometryInput>(alMesh);
	aiMaterial *pAiMaterial = alMesh.pAiMaterial;

	auto getTextureName = [&](aiTextureType type) -> std::optional<std::string> {
		if (pAiMaterial->GetTextureCount(type) == 0) {
			return std::nullopt;
		} else {
			aiString path;
			pAiMaterial->GetTexture(type, 0, &path);
			if (path.data[0] == '*') {					// 内嵌贴图,提取出索引
				size_t index = 0;
				sscanf_s("*%d", path.data, &index);
				return loader.getTextureName(index);
			} else {
				return std::string{ path.C_Str() };
			}
		}
	};

	if (pAiMaterial != nullptr) {
		if (auto texName = getTextureName(aiTextureType_DIFFUSE))
			_albedoMapName = *texName;
		if (auto texName = getTextureName(aiTextureType_METALNESS))
			_metallicMapName = *texName;
		if (auto texName = getTextureName(aiTextureType_DIFFUSE_ROUGHNESS))
			_roughnessMapName = *texName;
		if (auto texName = getTextureName(aiTextureType_NORMALS))
			_normalMapName = *texName;
		if (auto texName = getTextureName(aiTextureType_AMBIENT_OCCLUSION))
			_aoMapName = *texName;
	}
}

void StaticSubModel::prepareTexture(dx12lib::CommonContextProxy pCommonCtx) const {
	
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

void StaticModel::initAsAssimpLoader(const AssimpLoader &loader) {
	_subModels.clear();
	auto meshs = loader.parseMesh();
	for (AssimpLoader::ALMesh &mesh : meshs) {
		auto pSubMesh = std::make_shared<StaticSubModel>();
		pSubMesh->initAsALMesh(loader, mesh);
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