#pragma once
#include "ITick.h"
#include <wrl.h>
#include <d3d12.h>

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
private:
	WRL::ComPtr<ID3D12DescriptorHeap> _pDescriptorHeap;
};

}
