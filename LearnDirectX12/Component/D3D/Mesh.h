#pragma once
#include <memory>
#include <vector>
#include <d3d12.h>
#include <cassert>
#include <type_traits>
#include <string>
#include <unordered_map>
#include "Geometry/GeometryGenerator.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/CommandList.h"

namespace d3d {

// 如果 count 是 -1 表示无效 submesh
// 如果 startIndexLocation 是 -1, 表示为没有索引
struct SubMesh {
	std::string   _name;
	std::uint32_t _count;				
	std::uint32_t _startIndexLocation;
	std::uint32_t _baseVertexLocation;
public:
	explicit operator bool() const;
	void drawInstanced(dx12lib::CommandListProxy pCmdList,
		std::uint32_t instanceCount = 1, 
		std::uint32_t startInstanceLocation = 0
	) const;

	void drawIndexdInstanced(dx12lib::CommandListProxy pCmdList, 
		std::uint32_t instanceCount = 1, 
		std::uint32_t startInstanceLocation = 0
	) const;
};

class Mesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
	std::vector<SubMesh> _subMeshs;
public:
	Mesh &operator=(const Mesh &) = delete;
	Mesh(const Mesh &) = delete;
	Mesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer,
		std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer,
		const std::vector<SubMesh> &subMeshs = {}
	);

	void appendSubMesh(const SubMesh &submesh);

	std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer() const;
	std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer() const;
	
	using iteraotr = std::vector<SubMesh>::const_iterator;
	iteraotr begin() const;
	iteraotr end() const;

	void drawInstanced(dx12lib::CommandListProxy pCmdList,
		std::uint32_t instanceCount = 1,
		std::uint32_t startInstanceLocation = 0
	) const;

	void drawIndexdInstanced(dx12lib::CommandListProxy pCmdList,
		std::uint32_t instanceCount = 1,
		std::uint32_t startInstanceLocation = 0
	) const;

	SubMesh getSubmesh(const std::string &name) const;
};

enum class MeshIndexType {
	UINT8  = DXGI_FORMAT_R8_UINT,
	UINT16 = DXGI_FORMAT_R16_UINT,
	UINT32 = DXGI_FORMAT_R32_UINT,
};

template<MeshIndexType IndexType>
using MeshIndexTypeToIntegerType_t = std::conditional_t<(IndexType == MeshIndexType::UINT8),
	std::uint8_t,		
	std::conditional_t<(IndexType == MeshIndexType::UINT16),
		std::uint16_t,
		std::uint32_t
	>
>;

template<typename T, MeshIndexType IndexType = MeshIndexType::UINT16>
struct MakeMeshHelper {
	static std::shared_ptr<dx12lib::VertexBuffer> 
	buildVertexBuffer(dx12lib::CommandListProxy pCmdList, const com::MeshData &mesh) {
		if constexpr (std::is_same_v<com::Vertex, T>) {
			return pCmdList->createVertexBuffer(
				mesh.vertices.data(),
				sizeof(com::Vertex) * mesh.vertices.size(),
				sizeof(com::Vertex)
			);
		} else {
			std::vector<T> vertices;
			vertices.reserve(mesh.vertices.size());
			vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
			return pCmdList->createVertexBuffer(
				vertices.data(),
				sizeof(T) * vertices.size(),
				sizeof(T)
			);
		}
	}

	static std::shared_ptr<dx12lib::IndexBuffer>
	buildIndexBuffer(dx12lib::CommandListProxy pCmdList, const com::MeshData &mesh) {
		switch (IndexType) {
		case d3d::MeshIndexType::UINT8: {
			std::vector<std::uint8_t> indices;
			indices.reserve(mesh.indices.size());
			indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
			return pCmdList->createIndexBuffer(
				indices.data(),
				sizeof(std::uint8_t) * indices.size(),
				DXGI_FORMAT_R8_UINT
			);
			break;
		}
		case d3d::MeshIndexType::UINT16: {
			std::vector<std::uint16_t> indices;
			indices.reserve(mesh.indices.size());
			indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
			return pCmdList->createIndexBuffer(
				indices.data(),
				sizeof(std::uint16_t) * indices.size(),
				DXGI_FORMAT_R16_UINT
			);
			break;
		}
		case d3d::MeshIndexType::UINT32: {
			return pCmdList->createIndexBuffer(
				mesh.indices.data(),
				sizeof(std::uint32_t) * mesh.indices.size(),
				DXGI_FORMAT_R32_UINT
			);
			break;
		}
		default:
			assert(false && "error d3d::MeshIndexType type");
			break;
		}
		return nullptr;
	}

	static std::shared_ptr<Mesh> build(dx12lib::CommandListProxy pCmdList, 
		const com::MeshData &mesh, 
		const std::string &name = "") 
	{
		std::uint32_t count = -1;
		std::uint32_t vertCount = std::uint32_t(mesh.vertices.size());
		std::uint32_t indexCount = std::uint32_t(mesh.indices.size());
		if (!mesh.indices.empty() || !mesh.vertices.empty())
			count = (mesh.indices.empty()) ? vertCount : indexCount;

		assert(count != -1);
		auto pVertexBuffer = buildVertexBuffer(pCmdList, mesh);
		auto pIndexBuffer = buildIndexBuffer(pCmdList, mesh);
		SubMesh submesh = { name, count, 0, 0 };
		return std::make_shared<Mesh>(
			pVertexBuffer,
			pIndexBuffer,
			std::vector<SubMesh>({ submesh })
		);
	}
};


template<typename T, MeshIndexType IndexType = MeshIndexType::UINT16>
struct MakeUnionMeshHelper {
	void appendMesh(const std::string &name, const com::MeshData &mesh) {
		std::uint32_t count = -1;
		std::uint32_t vertCount = std::uint32_t(mesh.vertices.size());
		std::uint32_t indexCount = std::uint32_t(mesh.indices.size());
		if (!mesh.indices.empty() || !mesh.vertices.empty())
			count = (mesh.indices.empty()) ? vertCount : indexCount;

		assert(_subMeshs.find(name) == _subMeshs.end());
		assert(count != -1);

		SubMesh subMesh = {
			name,
			count,
			static_cast<UINT>(_indices.size()),
			static_cast<UINT>(_vertices.size()),
		};
		_vertices.insert(_vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		_indices.insert(_indices.end(), mesh.indices.begin(), mesh.indices.end());
		_subMeshs.emplace(name, subMesh);
	}

	std::shared_ptr<Mesh> build(dx12lib::CommandListProxy pCmdList) const {
		auto pVertexBuffer = pCmdList->createVertexBuffer(
			_vertices.data(),
			sizeof(T) * _vertices.size(),
			sizeof(T)
		);
		auto pIndexBuffer = pCmdList->createIndexBuffer(
			_indices.data(),
			sizeof(index_t) * _indices.size(),
			reinterpret_cast<DXGI_FORMAT>(IndexType)
		);

		std::vector<SubMesh> submesh;
		submesh.reserve(_subMeshs.size());
		submesh.insert(_subMeshs.begin(), _subMeshs.end());

		return std::make_shared<Mesh>(
			pVertexBuffer,
			pIndexBuffer,
			submesh
		);
	}
private:
	using index_t = MeshIndexTypeToIntegerType_t<IndexType>;
	std::vector<T> _vertices;
	std::vector<index_t> _indices;
	std::unordered_map<std::string, SubMesh> _subMeshs;
};

}