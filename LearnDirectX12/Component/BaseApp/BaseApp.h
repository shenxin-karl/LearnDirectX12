#pragma once
#define  NOMINMAX
#include <windows.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace com {

namespace DX  = DirectX;
namespace WRL = Microsoft::WRL;
class BaseApp {
public:
	BaseApp(HWND hwnd, int width, int height);
	BaseApp(const BaseApp &) = delete;
	BaseApp &operator=(const BaseApp &) = delete;
	virtual ~BaseApp() = default;
public:
	constexpr size_t kSwapChainCount = 2;
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

	D3D12_VIEWPORT	screenViewport_;
	D3D12_RECT		scissorRect_;

	HWND mainWindow_;
	UINT rtvDescriptorSize_ = 0;
	UINT dsvDescriptorSize_ = 0;
	UINT cbvSrvUavDescriptorSize_ = 0;
	bool massState_;
	UINT massQuality_;
	UINT currBackBuffer_ = 0;

	DXGI_FORMAT backBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
};

}