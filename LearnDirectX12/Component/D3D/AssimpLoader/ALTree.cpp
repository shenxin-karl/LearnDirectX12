#include "ALTree.h"
#include "ALNode.h"
#include <filesystem>

namespace d3d {

void ALMaterial::init(const std::string &direction, const aiScene *pAiScene, const aiMaterial *pAiMaterial) {
	processTexture(_diffuseMap, direction, pAiScene, pAiMaterial, aiTextureType_DIFFUSE);
	processTexture(_normalMap, direction, pAiScene, pAiMaterial, aiTextureType_NORMALS);
	processTexture(_specularMap, direction, pAiScene, pAiMaterial, aiTextureType_SPECULAR);
	processTexture(_smoothnessMap, direction, pAiScene, pAiMaterial, aiTextureType_SHININESS);
	processTexture(_metallicMap, direction, pAiScene, pAiMaterial, aiTextureType_METALNESS);
	processTexture(_ambientOcclusionMap, direction, pAiScene, pAiMaterial, aiTextureType_AMBIENT_OCCLUSION);
}

const ALTexture & ALMaterial::getDiffuseMap() const {
	return _diffuseMap;
}

const ALTexture & ALMaterial::getNormalMap() const {
	return _normalMap;
}

const ALTexture & ALMaterial::getSpecularMap() const {
	return _specularMap;
}

const ALTexture & ALMaterial::getSmoothnessMap() const {
	return _smoothnessMap;
}

const ALTexture & ALMaterial::getMetallicMap() const {
	return _metallicMap;
}

const ALTexture & ALMaterial::getAmbientOcclusionMap() const {
	return _ambientOcclusionMap;
}

void ALMaterial::processTexture(ALTexture &texture,
	const std::string &direction,
	const aiScene *pAiScene, 
	const aiMaterial *pAiMaterial, 
	aiTextureType type) 
{
	aiString path;
	pAiMaterial->GetTexture(type, 0, &path);
	const aiTexture *pAiTexture = pAiScene->GetEmbeddedTexture(path.C_Str());
	if (pAiTexture != nullptr) {
		texture.path = direction + path.C_Str();
		texture.textureDataSize = pAiTexture->mWidth;
		texture.textureExtName = pAiTexture->achFormatHint;
		if (pAiTexture->pcData != nullptr) {
			texture.pTextureData = std::make_shared<char[]>(pAiTexture->mWidth);
			std::memcpy(texture.pTextureData.get(), pAiTexture->pcData, pAiTexture->mWidth);
		}
	} else {
		texture.path = path.length > 0 ? direction + path.C_Str() : "";
	}
}

ALTree::ALTree(const std::string &path, int flag) {
	Assimp::Importer importer;
	const aiScene *pAiScene = importer.ReadFile(path, flag);
	if (pAiScene == nullptr || pAiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || pAiScene->mRootNode == nullptr) {
		assert(false);
		return;
	}

	namespace fs = std::filesystem;
	fs::path currPath(path);
	std::string direction = currPath.remove_filename().string();

	_materials.resize(pAiScene->mNumMaterials);
	for (size_t i = 0; i < pAiScene->mNumMeshes; ++i) {
		const aiMesh *pAiMesh = pAiScene->mMeshes[i];
		const aiMaterial *pAiMaterial = pAiScene->mMaterials[pAiMesh->mMaterialIndex];
		_materials[i].init(direction, pAiScene, pAiMaterial);
	}

	std::string_view modelPath(path.c_str(), path.length());
	_pRootNode = std::make_unique<ALNode>(this, modelPath, 0, pAiScene, pAiScene->mRootNode);
}

ALTree::~ALTree() = default;

size_t ALTree::getNumMaterial() const {
	return _materials.size();
}

const ALMaterial * ALTree::getMaterial(size_t idx) const {
	assert(idx < _materials.size());
	if (idx >= _materials.size())
		return nullptr;
	return &_materials[idx];
}

const ALNode * ALTree::getRootNode() const {
	return _pRootNode.get();
}

}