#pragma once
#include <string>
#include <unordered_map>
#include <Singleton/Singleton.hpp>
#include <Dx12lib/Buffer/VertexBuffer.h>
#include <Dx12lib/Buffer/IndexBuffer.h>

namespace d3d {

class MeshManager : public com::Singleton<MeshManager> {
public:
	std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer(const std::string &key) const;
	std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer(const std::string &key) const;
	void setVertexBuffer(const std::string &key, std::weak_ptr<dx12lib::VertexBuffer> pVertexBuffer);
	void setIndexBuffer(const std::string &key, std::weak_ptr<dx12lib::IndexBuffer> pIndexBuffer);
private:
	std::unordered_map<std::string, std::weak_ptr<dx12lib::VertexBuffer>> _vertexBufferCache;
	std::unordered_map<std::string, std::weak_ptr<dx12lib::IndexBuffer>> _indexBufferCache;
};

}
