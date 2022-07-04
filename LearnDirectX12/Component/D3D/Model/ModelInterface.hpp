#pragma once
#include <D3D/d3dutil.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace d3d {

interface IGeometryInput : public NonCopyable {
	virtual void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) = 0;
};

interface IInstanceInput : public NonCopyable {
	virtual void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) = 0;
};

interface ISubModel : public NonCopyable {
	virtual MaterialData getMaterialData() const = 0;
	virtual std::string getAOMapName() const = 0;
	virtual std::string getAlbedoMapName() const = 0;
	virtual std::string getNormalMapName() const = 0;
	virtual std::string getMetallicMapName() const = 0;
	virtual std::string getRoughnessMapName() const = 0;
	virtual std::shared_ptr<IGeometryInput> getGeometryInput() const = 0;
};

interface IModel : public NonCopyable {
	virtual const std::string &getModelName() const = 0;
	virtual size_t getSubModelCount() const = 0;
	virtual std::shared_ptr<ISubModel> getSubModel(size_t subId) const = 0;
};

interface IMaterial {
	virtual const std::string &getPSOName() const = 0;
	virtual void active(dx12lib::GraphicsContextProxy pGraphicsCtx) = 0;
};


}