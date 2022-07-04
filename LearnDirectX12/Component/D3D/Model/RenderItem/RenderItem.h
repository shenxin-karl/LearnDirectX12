#pragma once
#include <string>
#include <D3D/d3dutil.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace d3d {

struct DrawArgs {
	size_t vertexCount           = 0;
	size_t baseVertexLocation    = 0;
	size_t indexCount            = 0;
	size_t startIndexLocation    = 0;
	size_t instanceCount         = 1;
	size_t startInstanceLocation = 0;
};

class RenderItem : public NonCopyable {
public:
	RenderItem(const std::source_location &sr = std::source_location::current());
	void setPassName(const std::string &passName);
	void setGeometryInput(std::shared_ptr<IGeometryInput> pRenderInput);
	void setInstanceInput(std::shared_ptr<IInstanceInput> pInstanceInput);
	void setMaterial(std::shared_ptr<IMaterial> pMaterial);
	void setDrawArgs(const DrawArgs &drawArgs);
	const std::string &getPSOName() const;
	const std::string &getPassName() const;
	std::shared_ptr<IGeometryInput> getGeometryInput() const;
	std::shared_ptr<IInstanceInput> getInstanceInput() const;
	std::shared_ptr<IMaterial> getMaterial() const;
	const DrawArgs &getDrawArgs() const;
	void draw(dx12lib::GraphicsContextProxy pGraphicsCtx) const;
private:
	DrawArgs _drawArgs;
	std::string _passName;		
	std::source_location _sourceLocation;
	std::shared_ptr<IMaterial> _pMaterial;
	std::shared_ptr<IGeometryInput> _pGeometryInput;
	std::shared_ptr<IInstanceInput> _pInstanceInput;
};

}