#pragma once
#define  NOMINMAX
#include <windows.h>
#include <DirectXMath.h>
#include "dx12lib/dx12libStd.h"
#include "dx12lib/ContextProxy.hpp"
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
	virtual ~BaseApp() override = default;

	virtual void initialize() override;
	virtual void destroy() override;
	virtual void beginTick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void endTick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void resize(int width, int height);
	bool isRuning() const;
protected:
	virtual void onInitialize(dx12lib::DirectContextProxy pDirectCtx) {}
	virtual void onDistory() {}
	virtual void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {}
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) {}
	virtual void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {}
	virtual void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {}
protected:
	int  _width = 800;
	int  _height = 600;
	std::string _title = "BaseApp";
	std::shared_ptr<dx12lib::Adapter>   _pAdapter;
	std::shared_ptr<dx12lib::Device>    _pDevice;
	std::shared_ptr<dx12lib::SwapChain> _pSwapChain;
	std::unique_ptr<InputSystem>        _pInputSystem;
};

}