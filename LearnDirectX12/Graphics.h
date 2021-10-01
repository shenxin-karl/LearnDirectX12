#pragma once
#define  NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "ITick.h"
#include "ExceptionBase.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace WRL = Microsoft::WRL;
class Graphics : public ITick {
public:
	Graphics();
	virtual void tick() override {}
	virtual void update() {}
	virtual void draw() {}
	virtual void onResize() {}
	virtual void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {}
	void createCommandObjects();
	~Graphics() = default;
public:
	WRL::ComPtr<ID3D12Device>	d3dDevice_;
	WRL::ComPtr<IDXGIFactory1>	dxgiFactory_;
	WRL::ComPtr<ID3D12Fence>	fence_;
	WRL::ComPtr<ID3D12CommandQueue>			commandQueue_;
	WRL::ComPtr<ID3D12CommandAllocator>		directCmdListAlloc_;
	WRL::ComPtr<ID3D12GraphicsCommandList>	commandList_;
	UINT rtvDescriptorSize_;
	UINT dsvDescriptorSize_;
	UINT cbvUavDescriptorSize_;
	UINT msaaQualityLevel_;
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