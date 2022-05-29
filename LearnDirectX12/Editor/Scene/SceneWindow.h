#pragma once
#include <memory>
#include "D3D/dx12libHelper/RenderTarget.h"
#include "Editor/IEditorItem.h"

namespace ED {

class SceneWindow : public IEditorWindow {
public:
	void showWindow() override;
	bool *getOpenFlagPtr() override;
	void resizeRenderTarget(size_t width, size_t height, dx12lib::DirectContextProxy pDirectCtx);
private:
	bool _show = true;
	size_t _width = 0;
	size_t _height = 0;
	std::shared_ptr<dx12lib::RenderTarget2D> _pRenderTarget2D;
	std::shared_ptr<dx12lib::DepthStencil2D> _pDepthStencil2D;
};

}
