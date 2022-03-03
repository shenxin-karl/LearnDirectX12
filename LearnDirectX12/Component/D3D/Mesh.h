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

struct Mesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
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
struct MakeMeshHelper;

template<typename T, MeshIndexType IndexType>
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

	static std::shared_ptr<Mesh> build(dx12lib::CommandListProxy pCmdList, const com::MeshData &mesh) {
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>();
		pMesh->_pVertexBuffer = buildVertexBuffer(pCmdList, mesh);
		pMesh->_pIndexBuffer = buildIndexBuffer(pCmdList, mesh);
		return pMesh;
	}
};

struct SubMesh {
	UINT indexCount;
	UINT startIndexLocation;
	UINT baseVertexLocation;
};

struct UnionMesh : public Mesh {
	UnionMesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer,
		std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer,
		const std::unordered_map<std::string, SubMesh> &subMeshs
	);
	UnionMesh(const UnionMesh &) = delete;
	UnionMesh &operator=(const UnionMesh &) = delete;
	bool hasSubMesh(const std::string &name, SubMesh &subMesh) const;
public:
	const std::unordered_map<std::string, SubMesh> _subMeshs;
};

template<typename T, MeshIndexType IndexType = MeshIndexType::UINT16>
struct MakeUnionMeshHelper {
	void appendMesh(const std::string &name, const com::MeshData &mesh) {
		assert(_subMeshs.find(name) == _subMeshs.end());
		SubMesh subMesh = {
			static_csat<UINT>(mesh.indices.size()),
			static_cast<UINT>(_indices.size()),
			static_cast<UINT>(_vertices.size()),
		};
		_vertices.insert(_vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		_indices.insert(_indices.end(), mesh.indices.begin(), mesh.indices.end());
		_subMeshs.emplace(name, subMesh);
	}

	std::shared_ptr<UnionMesh> build(dx12lib::CommandListProxy pCmdList) const {
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

		return std::make_shared<UnionMesh>(
			pVertexBuffer,
			pIndexBuffer,
			_subMeshs
		);
	}
private:
	using index_t = MeshIndexTypeToIntegerType_t<IndexType>;
	std::vector<T> _vertices;
	std::vector<index_t> _indices;
	std::unordered_map<std::string, SubMesh> _subMeshs;
};

}