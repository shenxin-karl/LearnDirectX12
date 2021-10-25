#pragma once
#include "Graphics.h"
#include "UploadBuffer.h"
#include "MathHelper.h"

namespace WRL = Microsoft::WRL;
namespace DX = DirectX;

struct ObjectConstant {
	DX::XMFLOAT4X4	gWorldViewProj;
};

class TestGraphics : public Graphics {
	using Base = Graphics;
public:
	TestGraphics() = default;
	virtual void initialize() override;
	virtual void tick(GameTimer &dt) override;
	virtual void update() override;
	virtual void draw() override;
	virtual void onResize() override;
	virtual void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual ~TestGraphics() override = default;
private:
	WRL::ComPtr<ID3D12RootSignature>	 rootSignature_ = nullptr;
	WRL::ComPtr<ID3D12DescriptorHeap>	 cbvHeap_ = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstant>> objectCB_ = nullptr;
	std::unique_ptr<MeshGeometry>				  objectGeometry_ = nullptr;
	WRL::ComPtr<ID3DBlob>						  vsByteCode_ = nullptr;
	WRL::ComPtr<ID3DBlob>						  fsByteCode_ = nullptr;
	WRL::ComPtr<ID3D12PipelineState>			  pos_ = nullptr;
	std::vector<D3D12_INPUT_LAYOUT_DESC>		  inputLayout_;
	DX::XMFLOAT4X4 model_ = MathHelper::identity4x4();
	DX::XMFLOAT4X4 view_ = MathHelper::identity4x4();
	DX::XMFLOAT4X4 project_ = MathHelper::identity4x4();

	float theta_ = 1.5f * DX::XM_PI;
	float phi_ = DX::XM_PIDIV4;
	float radius_ = 5.f;

	POINT lastMousePos_;
};

