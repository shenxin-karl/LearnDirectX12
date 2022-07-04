#pragma once
#include <D3D/d3dutil.h>
#include <D3D/Model/ModelInterface.hpp>
#include <D3D/AssimpLoader/AssimpLoader.h>

namespace d3d {

class StaticGeometryInput : public IGeometryInput {
public:
	StaticGeometryInput(dx12lib::GraphicsContextProxy pGraphicsCtx, const AssimpLoader::ALMesh &mesh);
	void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) override;
	std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer() const;
	std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer() const;
private:
	std::shared_ptr<dx12lib::IndexBuffer> _pIndexBuffer;
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
};

}