#pragma once
#include "Core/Scene/SceneManager.h"
#include "Editor/ImGuiProxy/ImGuiInputFilter.h"

namespace ED {

class ISceneNodeEditor;

class SceneManagerEditor : public core::SceneManager {
public:
	SceneManagerEditor();
	~SceneManagerEditor();
	void drawSceneWindow();
	void drawHierarchyWindow();
	void drawInspectorWindow();
	void drawLightingWindow();
private:
	void resizeRenderTarget(size_t width, size_t height, dx12lib::GraphicsContextProxy pGraphicsCtx);
	void handleImGuiInput();
public:
	bool showSceneWindow	 = true;
	bool showLightWindow     = true;
	bool showHierarchyWindow = true;
	bool showInspectorWindow = true;
private:
	size_t _sceneWidth  = 0;
	size_t _sceneHeight = 0;
	ED::ImGuiInputFilter _sceneWindowInputFilter;
	std::shared_ptr<ISceneNodeEditor> _pSelectedSceneNode;
	std::shared_ptr<dx12lib::RenderTarget2D> _pRenderTarget2D;
	std::shared_ptr<dx12lib::DepthStencil2D> _pDepthStencil2D;
};

}
