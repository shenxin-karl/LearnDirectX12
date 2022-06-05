#pragma once
#include "ITick.h"
#include <wrl.h>
#include <d3d12.h>
#include "Editor/IEditorItem.h"
#include "Imgui/imgui.h"
#include "Resource/ResourceView.hpp"

namespace dx12lib {
	class Device;
}

namespace ImGui {

namespace WRL = Microsoft::WRL;

class ImGuiProxy : public com::ITick {
public:
	void initialize() override;
	void destroy() override;
	void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void endTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
public:
	enum ImGuiSrvSlot : size_t {
		Default = 0,
		Scene   = 1,
		Count,
	};
	void image(ImGuiSrvSlot slot, dx12lib::ShaderResourceView srv, size_t width, size_t height);
private:
	void showAppDockSpace();
private:
	int  _styleIndex = 1;
	bool _enableDockSpace = true;
	bool _optPadding = false;
	ImGuiDockNodeFlags _dockSpaceFlag = ImGuiDockNodeFlags_None;
	size_t _descriptorIncrementSize = 0;
	WRL::ComPtr<ID3D12DescriptorHeap> _pDescriptorHeap;
};

}
