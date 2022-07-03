#include "TextureManager.h"

namespace d3d {

std::shared_ptr<dx12lib::IShaderResource> TextureManager::get(const std::string &fileName) const {
	auto iter = _textureMap.find(fileName);
	if (iter != _textureMap.end())
		return iter->second;
	return nullptr;
}

bool TextureManager::exist(const std::string &fileName) const {
	auto iter = _textureMap.find(fileName);
	return iter != _textureMap.end();
}

void TextureManager::set(const std::string &fileName, std::shared_ptr<dx12lib::IShaderResource> pShaderResource) {
	assert(!fileName.empty());
	assert(pShaderResource != nullptr);
	_textureMap[fileName] = pShaderResource;
}

void TextureManager::erase(const std::string &fileName) {
	_textureMap.erase(fileName);
}

void TextureManager::initDefaultTexture(dx12lib::DirectContextProxy pGraphicsCtx) {

}

}