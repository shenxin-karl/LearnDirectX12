#pragma once
#include <memory>
#include "Graphics.h"
#include "UploadBuffer.h"
#include "MathHelper.h"

namespace WRL = Microsoft::WRL;
namespace DX = DirectX;

struct BoxObjectConstant {
	DX::XMFLOAT4X4	gWorldViewProj;
};

struct BoxVertex {
	DX::XMFLOAT3 position;
	DX::XMFLOAT4 color;
};

class TestGraphics : public Graphics {
	using Base = Graphics;
public:
	TestGraphics() = default;
	virtual bool initialize() override;
	virtual void tick(GameTimer &dt) override;
	virtual void update() override;
	virtual void draw() override;
	virtual void onResize() override;
	virtual void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
	void handleMouseMsg();
	void onMouseMove(POINT p);
	void onMouseLPress(POINT p);
	void onMouseLRelese(POINT p);
	virtual ~TestGraphics() override = default;
	void buildDescriptorHeaps();
	void buildConstantBuffers();
	void buildRootSignature();
	void buildShaderAndInputLayout();
	void buildBoxGeometry();
	void buildPSO();
	void updateView();
	void updateProj();
private:
	WRL::ComPtr<ID3D12RootSignature>	 rootSignature_ = nullptr;
	WRL::ComPtr<ID3D12DescriptorHeap>	 cbvHeap_ = nullptr;
	std::unique_ptr<UploadBuffer<BoxObjectConstant>> objectCB_ = nullptr;
	std::unique_ptr<MeshGeometry>					objectGeometry_ = nullptr;
	WRL::ComPtr<ID3DBlob>							vsByteCode_ = nullptr;
	WRL::ComPtr<ID3DBlob>							psByteCode_ = nullptr;
	WRL::ComPtr<ID3D12PipelineState>				pso_ = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC>			inputLayout_;
	DX::XMFLOAT4X4 model_ = MathHelper::identity4x4();
	DX::XMFLOAT4X4 view_ = MathHelper::identity4x4();
	DX::XMFLOAT4X4 project_ = MathHelper::identity4x4();

	float theta_ = 1.5f * DX::XM_PI;
	float phi_ = DX::XM_PIDIV4;
	float radius_ = 15.f;
	bool isMouseLPress_ = false;
	POINT lastMousePos_;
};

