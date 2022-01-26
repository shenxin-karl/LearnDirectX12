#pragma once
#define  NOMINMAX
#include <windows.h>
#include <DirectXMath.h>
#include "dx12lib/dx12libStd.h"
#include "ITick.h"
#include "InputSystem/InputSystem.h"


namespace com {

namespace DX  = DirectX;
namespace WRL = Microsoft::WRL;
class InputSystem;

class BaseApp : public ITick {
public:
	BaseApp() = default;
	BaseApp(const BaseApp &) = delete;
	BaseApp &operator=(const BaseApp &) = delete;
	virtual ~BaseApp() = default;

	virtual bool initialize();
	virtual void beginTick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void endTick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height);
	bool isRuning() const;
protected:
	int _width = 800;
	int _height = 600;
	std::string _title = "BaseApp";
	DXGI_FORMAT _backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT _depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	std::shared_ptr<dx12lib::Adapter>   _pAdapter;
	std::shared_ptr<dx12lib::Device>    _pDevice;
	std::shared_ptr<dx12lib::SwapChain> _pSwapChain;
	std::unique_ptr<InputSystem>        _pInputSystem;
};

}