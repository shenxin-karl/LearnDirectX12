#pragma once
#include <memory>
#include <vector>
#include <d3d12.h>
#include <cassert>
#include <type_traits>
#include <string>
#include <unordered_map>
#include "Geometry/GeometryGenerator.h"
#include <dx12lib/Context/ContextProxy.hpp>
#include <DirectXCollision.h>

namespace d3d {

// 如果 count 是 -1 表示无效 submesh
// 如果 startIndexLocation 是 -1, 表示为没有索引
struct SubMesh {
	inline static std::string invalidSubMeshName = "Invalid";
	std::string name			   = invalidSubMeshName;
	std::size_t count			   = -1;				
	std::size_t startIndexLocation = -1;
	std::size_t baseVertexLocation = -1;
public:
	explicit operator bool() const;
	void drawInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
		size_t instanceCount = 1,
		size_t startInstanceLocation = 0
	) const;

	void drawIndexedInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
		size_t instanceCount = 1,
		size_t startInstanceLocation = 0
	) const;
};

class Mesh {
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::shared_ptr<dx12lib::IndexBuffer>  _pIndexBuffer;
	std::vector<SubMesh> _subMeshs;
	DX::BoundingBox _bounds;
public:
	Mesh &operator=(const Mesh &) = delete;
	Mesh(const Mesh &) = delete;
	Mesh(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer,
		std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer,
		const DX::BoundingBox &bounds = DX::BoundingBox(),
		const std::vector<SubMesh> &subMeshs = {}
	);

	void appendSubMesh(const SubMesh &subMesh);

	std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer() const;
	std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer() const;
	
	using iteraotr = std::vector<SubMesh>::const_iterator;
	iteraotr begin() const;
	iteraotr end() const;

	void drawInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
		size_t instanceCount = 1,
		size_t startInstanceLocation = 0
	) const;

	void drawIndexedInstanced(dx12lib::GraphicsContextProxy pGraphicsCtx,
		size_t instanceCount = 1,
		size_t startInstanceLocation = 0
	) const;

	SubMesh getSubMesh(const std::string &name) const;
	SubMesh getSubMesh() const;

	const DX::BoundingBox &getBounds() const;
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

template<typename T = com::Vertex, MeshIndexType IndexType = MeshIndexType::UINT16>
struct MakeMeshHelper {
	static std::shared_ptr<dx12lib::VertexBuffer> 
	buildVertexBuffer(dx12lib::GraphicsContextProxy pGrahpicsCtx, const com::MeshData &mesh) {
		if constexpr (std::is_same_v<com::Vertex, T>) {
			return pGrahpicsCtx->createVertexBuffer(
				mesh.vertices.data(),
				mesh.vertices.size(),
				sizeof(com::Vertex)
			);
		} else {
			std::vector<T> vertices;
			vertices.reserve(mesh.vertices.size());
			vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
			return pGrahpicsCtx->createVertexBuffer(
				vertices.data(),
				vertices.size(),
				sizeof(T)
			);
		}
	}

	static std::shared_ptr<dx12lib::IndexBuffer>
	buildIndexBuffer(dx12lib::GraphicsContextProxy pGrahpicsCtx, const com::MeshData &mesh) {
		switch (IndexType) {
		case d3d::MeshIndexType::UINT8: {
			std::vector<std::uint8_t> indices;
			indices.reserve(mesh.indices.size());
			indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
			return pGrahpicsCtx->createIndexBuffer(
				indices.data(),
				indices.size(),
				DXGI_FORMAT_R8_UINT
			);
			break;
		}
		case d3d::MeshIndexType::UINT16: {
			std::vector<std::uint16_t> indices;
			indices.reserve(mesh.indices.size());
			indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
			return pGrahpicsCtx->createIndexBuffer(
				indices.data(),
				indices.size(),
				DXGI_FORMAT_R16_UINT
			);
			break;
		}
		case d3d::MeshIndexType::UINT32: {
			return pGrahpicsCtx->createIndexBuffer(
				mesh.indices.data(),
				mesh.indices.size(),
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

	static std::shared_ptr<Mesh> build(dx12lib::GraphicsContextProxy pGrahpicsCtx,
		const com::MeshData &mesh,
		const std::string &name = "") {
		std::uint32_t count = -1;
		std::uint32_t vertCount = static_cast<std::uint32_t>(mesh.vertices.size());
		std::uint32_t indexCount = static_cast<std::uint32_t>(mesh.indices.size());
		if (!mesh.indices.empty() || !mesh.vertices.empty())
			count = (mesh.indices.empty()) ? vertCount : indexCount;

		assert(count != -1);
		auto pVertexBuffer = buildVertexBuffer(pGrahpicsCtx, mesh);
		auto pIndexBuffer = buildIndexBuffer(pGrahpicsCtx, mesh);

		const auto *pVertex = mesh.vertices.data();
		DX::BoundingBox bounds;
		DX::BoundingBox::CreateFromPoints(bounds, mesh.vertices.size(), &pVertex->position, sizeof(com::Vertex));

		std::vector<SubMesh> submeshs;
		if (!name.empty())
			submeshs.emplace_back(name, count, static_cast<std::uint32_t>(0), static_cast<std::uint32_t>(0));

		return std::make_shared<Mesh>(
			pVertexBuffer,
			pIndexBuffer,
			bounds,
			submeshs
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

		const auto *pVertex = mesh.vertices.data();
		DX::BoundingBox submeshBounds;
		auto tempBounds = _bounds;
		DX::BoundingBox::CreateFromPoints(submeshBounds, mesh.vertices.size(), &pVertex->position, sizeof(com::Vertex));
		DX::BoundingBox::CreateMerged(_bounds, tempBounds, submeshBounds);

		_subMeshs.emplace(name, subMesh);
	}

	std::shared_ptr<Mesh> build(dx12lib::GraphicsContextProxy pGrahpicsCtx) const {
		auto pVertexBuffer = pGrahpicsCtx->createVertexBuffer(
			_vertices.data(),
			_vertices.size(),
			sizeof(T)
		);
		auto pIndexBuffer = pGrahpicsCtx->createIndexBuffer(
			_indices.data(),
			_indices.size(),
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
	DX::BoundingBox _bounds;
};

}