#pragma once
#include <D3D/d3dutil.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include <D3D/Model/Material/Material.h>

namespace d3d {

interface ISubModel : NonCopyable {
	virtual const d3d::Material &getMaterial() const = 0;
	virtual std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer() const = 0;
	virtual std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer() const = 0;
};

interface IModel : NonCopyable {
	virtual const std::string &getModelName() const = 0;
	virtual size_t getSubModelCount() const = 0;
	virtual std::shared_ptr<ISubModel> getSubModel(size_t subId) const = 0;
};


}