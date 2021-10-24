#pragma once
#define  NOMINMAX
#include <windows.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "d3dx12.h"
#include "d3dulti.h"
#include "ITick.h"
#include "ExceptionBase.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace WRL = Microsoft::WRL;
class Graphics : public ITick {
public:
	Graphics() = default;
	virtual void initialize();
	virtual void tick(GameTimer &dt) override {}
	virtual void update() {}
	virtual void draw() {}
	virtual void onResize();
	virtual void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {}
	virtual ~Graphics();
	virtual void createRtvAndDsvDescriptorHeaps();
	void createCommandObjects();
	void createSwapChain();
	void flushCommandQueue();
	void calculateFrameStats();
	void set4xMsaaState(bool val);
	ID3D12Resource *currentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView() const;
protected:
	static constexpr int kSwapChainCount = 2;
	WRL::ComPtr<ID3D12Device>				d3dDevice_;
	WRL::ComPtr<IDXGIFactory1>				dxgiFactory_;
	WRL::ComPtr<ID3D12Fence>				fence_;
	WRL::ComPtr<ID3D12CommandQueue>			commandQueue_;
	WRL::ComPtr<ID3D12CommandAllocator>		directCmdListAlloc_;
	WRL::ComPtr<ID3D12GraphicsCommandList>	commandList_;
	WRL::ComPtr<IDXGISwapChain>				swapChain_;
	WRL::ComPtr<ID3D12DescriptorHeap>		rtvHeap_;
	WRL::ComPtr<ID3D12DescriptorHeap>		dsvHeap;
	WRL::ComPtr<ID3D12Resource>				depthStencilBuffer_;
	WRL::ComPtr<ID3D12Resource>				swapChainBuffer_[kSwapChainCount];
	RECT									scissiorRect_ = {};
	D3D12_VIEWPORT							screenViewport_ = {};
	UINT rtvDescriptorSize_ = 0;
	UINT dsvDescriptorSize_ = 0;
	UINT cbvUavDescriptorSize_ = 0;
	UINT msaaQualityLevel_ = 0;
	UINT64 currentFence_ = 0;
	bool msaaState_ = false;
	int  currBackBuffer_ = 0;
protected:
	D3D_DRIVER_TYPE	d3dDeriverType		= D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT		backBufferFormat_   = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT		depthStencilFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
};

class GraphicsException : public ExceptionBase {
public:
	GraphicsException(const char *file, int line, HRESULT hr);
	const char *what() const noexcept override;
	virtual const char *getType() const noexcept;
	std::string getErrorString() const;
public:
	HRESULT	hr_;
};