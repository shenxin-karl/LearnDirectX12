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
	void initAsALMesh(const AssimpLoader &loader, const AssimpLoader::ALMesh &alMesh);
	MaterialData getMaterialData() const override;
	std::string getAOMapName() const override;
	std::string getAlbedoMapName() const override;
	std::string getNormalMapName() const override;
	std::string getMetallicMapName() const override;
	std::string getRoughnessMapName() const override;
	std::shared_ptr<IGeometryInput> getGeometryInput() const override;
private:
	MaterialData _materialData    = MaterialData::defaultMaterialData;
	std::string _aoMapName        = TextureManager::defaultAOMap;
	std::string _albedoMapName    = TextureManager::defaultAlbedoMap;
	std::string _normalMapName    = TextureManager::defaultNormalMap;
	std::string _roughnessMapName = TextureManager::defaultRoughnessMap;
	std::string _metallicMapName  = TextureManager::defaultMetallicMap;
	std::shared_ptr<IGeometryInput> _pGeometryInput;
};

class StaticModel : public IModel {
public:
	StaticModel(const std::string &modelName);
	void initAsAssimpLoader(const AssimpLoader &loader);
	const std::string &getModelName() const override;
	size_t getSubModelCount() const override;
	std::shared_ptr<ISubModel> getSubModel(size_t subId) const override;
private:
	std::string _modelName;
	std::vector<std::shared_ptr<StaticSubModel>> _subModels;
};

}