#pragma once
#include "Core/Scene/SceneManager.h"

namespace ED {

class SceneManagerEditor : public core::SceneManager {
public:
	SceneManagerEditor() = default;
	void drawSceneWindow();
	void drawHierarchyWindow();
	void drawInspectorWindow();
private:
	void resizeRenderTarget(size_t width, size_t height, dx12lib::GraphicsContextProxy pGraphicsCtx);
public:
	bool showSceneWindow	 = true;
	bool showHierarchyWindow = true;
	bool showInspectorWindow = true;
private:
	size_t _sceneWidth  = 0;
	size_t _sceneHeight = 0;
	std::shared_ptr<dx12lib::RenderTarget2D> _pRenderTarget2D;
	std::shared_ptr<dx12lib::DepthStencil2D> _pDepthStencil2D;
};

}
