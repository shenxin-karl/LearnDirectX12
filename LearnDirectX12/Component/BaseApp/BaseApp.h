#pragma once
#define  NOMINMAX
#include <windows.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <memory>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <string>
#include "ITick.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace com {

namespace DX  = DirectX;
namespace WRL = Microsoft::WRL;
class InputSystem;

class BaseApp : public ITick {
public:
	BaseApp(const BaseApp &) = delete;
	BaseApp &operator=(const BaseApp &) = delete;
	virtual bool initialize();
	virtual void beginTick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void endTick(std::shared_ptr<GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height);
	virtual ~BaseApp() = default;
public:
	constexpr static size_t kSwapChainCount = 2;
	void creaetSwapChain();
	UINT getSampleCount() const;
	UINT getSampleQuality() const;
	void flushCommandQueue();
private:
	bool initializeD3D();
	void createCommandObjects();
	void createRtvAndDsvDescriptorHeaps();
protected:
	WRL::ComPtr<ID3D12Device>		pDevice_;
	WRL::ComPtr<ID3D12Fence>		pFence_;
	WRL::ComPtr<IDXGIFactory4>		pDxgiFactory_;
	WRL::ComPtr<IDXGISwapChain>		pSwapChain_;
	WRL::ComPtr<ID3D12Resource>		pSwapChainBuffer_[kSwapChainCount];
	WRL::ComPtr<ID3D12Resource>		pDepthStencilBuffer_;

	WRL::ComPtr<ID3D12GraphicsCommandList>	pCommandList_;
	WRL::ComPtr<ID3D12CommandQueue>			pCommandQueue_;
	WRL::ComPtr<ID3D12CommandAllocator>		pCommandAlloc_;

	WRL::ComPtr<ID3D12DescriptorHeap>		pRtvHeap_;
	WRL::ComPtr<ID3D12DescriptorHeap>		pDsvHeap_;
	std::unique_ptr<InputSystem>			pInputSystem_;

	D3D12_VIEWPORT	screenViewport_;
	D3D12_RECT		scissorRect_;

	UINT rtvDescriptorSize_ = 0;
	UINT dsvDescriptorSize_ = 0;
	UINT cbvSrvUavDescriptorSize_ = 0;
	bool msaaState_ = false;
	UINT msaaQuality_ = 0;
	UINT currBackBuffer_ = 0;

protected:
	DXGI_FORMAT backBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
	std::string title_  = "BaseApp";
	int			width_  = 800;
	int			height_ = 600;
};

}