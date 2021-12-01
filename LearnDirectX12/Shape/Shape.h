#pragma once
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "Geometry/GeometryGenerator.h"
#include "D3D/d3dulti.h"
#include "D3D/RenderItem.h"
#include "D3D/UploadBuffer.h"
#include "D3D/FrameResource.h"
#include <unordered_map>
#include <wrl.h>

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

using namespace vec;
using namespace matrix;

struct ShapeVertex {
	float3	position;
	float4	color;
};

struct ShaderByteCode {
	WRL::ComPtr<ID3DBlob> pVsByteCode;
	WRL::ComPtr<ID3DBlob> pPsByteCode;
public:
	D3D12_SHADER_BYTECODE getVsByteCode() const;
	D3D12_SHADER_BYTECODE getPsByteCode() const;
};

class Shape : public com::BaseApp {
public:
	virtual bool initialize();
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void endTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height) override;
	virtual ~Shape() override;
private:
	void buildShapeGeometry();
	void buildRenderItems();
	void buildDescriptorHeaps();
	void buldConstantBufferViews();
	void buildShaderAndInputLayout();
	void buildRootSignature();
	void buildPSO();
	void updateObjectConstant();
	void updatePassConstant();
private:
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometrice_;
	std::unordered_map<std::string, ShaderByteCode> shaders_;
	std::unordered_map<std::string, WRL::ComPtr<ID3D12PipelineState>> PSOs_;
	std::vector<std::unique_ptr<d3dUlti::RenderItem>> allRenderItems_;
	std::vector<d3dUlti::RenderItem *> opaqueRItems_;
	WRL::ComPtr<ID3D12DescriptorHeap> pCbvHeaps_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;
	WRL::ComPtr<ID3D12RootSignature> pRootSignature;

	UINT passCbvOffset_ = 0;
	bool isWireframe_ = false;
	POINT lastMousePos_;
	PassConstants mainPassCB;

	float theta_ = 0.f;
	float phi_ = 0.f;
	float radius_ = 15.f;

	float3	eyePos_;
	float4x4 view_;
	float4x4 proj_;

	std::vector<std::unique_ptr<FrameResource>> frameResources_;
	UINT currentFrameIndex_ = 0;
	FrameResource *currentFrameResource_ = nullptr;
};