#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include <dx12lib/Texture/TextureStd.h>
#include <Singleton/Singleton.hpp>
#include <D3D/AssimpLoader/AssimpLoader.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace d3d {

class TextureManager : public com::Singleton<TextureManager> {
public:
	std::shared_ptr<dx12lib::IShaderResource> get(const std::string &fileName) const;
	bool exist(const std::string &fileName) const;
	void set(const std::string &fileName, std::shared_ptr<dx12lib::IShaderResource> pShaderResource);
	void erase(const std::string &fileName);
public:
	static inline std::string defaultAOMap        = "DefaultAlbedoMap.dds";
	static inline std::string defaultAlbedoMap    = "DefaultAlbedoMap.dds";
	static inline std::string defaultNormalMap    = "DefaultNormalMap.dds";
	static inline std::string defaultMetallicMap  = "DefaultMetallicMap.dds";
	static inline std::string defaultRoughnessMap = "DefaultRoughnessMap.dds";
	static void initDefaultTexture(dx12lib::DirectContextProxy pGraphicsCtx);
private:
	std::unordered_map<std::string, std::shared_ptr<dx12lib::IShaderResource>> _textureMap;
};

}