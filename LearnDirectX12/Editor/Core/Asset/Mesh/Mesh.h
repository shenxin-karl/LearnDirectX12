#pragma once
#include <Core/Asset/IAsset.hpp>

namespace core {

struct MeshMetaDesc {
	std::string path;
};

class Mesh : public IAsset {
public:
	Mesh(const std::string &importPath, const std::string &savePath);
	Mesh(const MeshMetaDesc &desc);
	virtual const std::string &getAssetName() const = 0;
	virtual const std::string &getAssetPath() const = 0;
	virtual void saveMeta() const = 0;
	const std::string &getType() const;
	size_t getVertexCount() const;
	size_t getIndexCount() const;
	bool hasNormal() const;
	bool hasUV0() const;
	bool hasUV1() const;
	bool hasTangent() const;
	bool hasBoneWeight() const;
private:
	bool _hasNormal;
	bool _hasUV0;
	bool _hasUV1;
	bool _hasTangent;
	bool _hasBoneWeight;
	size_t _indexCount;
	size_t _vertexCount;
	std::string _type;
	std::string _meshName;
	std::string _meshPath;
};

}
