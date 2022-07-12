#pragma once
#include <memory>
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextStd.h>
#include <RenderGraph/Technique/TechniqueType.hpp>

namespace rg {

struct DrawArgs {
	size_t vertexCount = 0;
	size_t baseVertexLocation = 0;
	size_t indexCount = 0;
	size_t startIndexLocation = 0;
	size_t instanceCount = 1;
	size_t startInstanceLocation = 0;
};

class Drawable : public NonCopyable {
public:
	Drawable() = default;
	~Drawable() override = default;
	virtual void bind(dx12lib::IGraphicsContext &graphicsCtx) const;
	void submit(TechniqueType filter);
	void addTechnique(std::unique_ptr<Technique> pTechnique);
	void remoteTechnique(const std::string &techniqueName);
	TechniqueType *getTechnique(const std::string &techniqueName) const;
	void setDrawArgs(const DrawArgs &drawArgs);
	void setTopology(D3D_PRIMITIVE_TOPOLOGY topology);
	void setIndexBuffer(std::shared_ptr<dx12lib::IndexBuffer> pIndexBuffer);
	void setVertexBuffer(std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer);
	const DrawArgs &getDrawArgs() const;
	D3D_PRIMITIVE_TOPOLOGY getTopology() const;
	std::shared_ptr<dx12lib::IndexBuffer> getIndexBuffer() const;
	std::shared_ptr<dx12lib::VertexBuffer> getVertexBuffer() const;
private:
	DrawArgs _drawArgs;
	D3D_PRIMITIVE_TOPOLOGY _topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	std::shared_ptr<dx12lib::IndexBuffer> _pIndexBuffer;
	std::shared_ptr<dx12lib::VertexBuffer> _pVertexBuffer;
	std::vector<std::unique_ptr<Technique>> _techniques;
};

}
