#pragma once
#include "BaseApp/BaseApp.h"
#include "GameTimer/GameTimer.h"
#include "Geometry/GeometryGenerator.h"
#include "D3D/d3dutil.h"
#include "D3D/RenderItem.h"
#include "D3D/UploadBuffer.h"
#include "D3D/FrameResource.h"
#include "D3D/Texture.h"
#include <unordered_map>
#include <wrl.h>

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

using namespace vec;
using namespace matrix;

struct ColorVertex {
	float3	position;
	float3  normal;
};

struct TextureVertex {
	float3 position;
	float3 normal;
	float2 texcoord;
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
	virtual bool initialize() override;
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void onResize(int width, int height) override;
	virtual ~Shape() override;
private:
	void pollEvent();
	void buildFrameResources();
	void buildShapeGeometry();
	void buildRenderItems();
	void buildDescriptorHeaps();
	void buldConstantBufferViews();
	void buildShaderAndInputLayout();
	void buildRootSignatureImpl(const std::vector<CD3DX12_ROOT_PARAMETER> &rootParam, 
		WRL::ComPtr<ID3D12RootSignature> &pRootSignature);
	void buildColorRootSignature();
	void buildTextureRootSignature();
	void buildMaterials();
	void buildPSO();
	void loadTexture();
	void updateObjectConstant();
	void updatePassConstant(std::shared_ptr<com::GameTimer> pGameTimer);
	void updateMaterials();
	void drawColorRenderItem();
	void drawTextureRenderItem();
	void updateViewMatrix();
private:
	void onKeyDown(char key);
	void onMouseWhell(float offset);
	void onMouseMove(POINT point);
	void onMouseLPress(POINT point);
	void onMouseLRelease(POINT point);
private:
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometrice_;
	std::unordered_map<std::string, ShaderByteCode> shaders_;
	std::unordered_map<std::string, WRL::ComPtr<ID3D12PipelineState>> PSOs_;
	std::unordered_map<std::string, std::unique_ptr<d3dUtil::Material>> materials_;
	std::unordered_map<std::string, std::unique_ptr<d3dUtil::Texture>> textures_;
	std::vector<std::unique_ptr<d3dUtil::RenderItem>> colorRenderItems_;
	std::vector<std::unique_ptr<d3dUtil::RenderItem>> textureRenderItems_;
	std::vector<d3dUtil::RenderItem *> opaqueRItems_;
	WRL::ComPtr<ID3D12DescriptorHeap> pCbvHeaps_;
	WRL::ComPtr<ID3D12DescriptorHeap> pSrvHeaps_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> colorInputLayout_;
	std::vector<D3D12_INPUT_ELEMENT_DESC> textureInputLayout_;
	WRL::ComPtr<ID3D12RootSignature> pColorRootSignature_;
	WRL::ComPtr<ID3D12RootSignature> pTextureRootSignature_;

	UINT passCbvOffset_ = 0;
	bool isWireframe_ = false;
	POINT lastMousePos_;
	bool isLeftPressd = false;
	d3dUtil::PassConstants mainPassCB_;

	float theta_ = 0.f;
	float phi_ = 0.f;
	float radius_ = 15.f;

	float3	eyePos_;
	float4x4 view_;
	float4x4 proj_;

	const float zNear = 0.1f;
	const float zFar = 100.f;

	std::vector<std::unique_ptr<d3dUtil::FrameResource>> frameResources_;
	UINT currentFrameIndex_ = 0;
	d3dUtil::FrameResource *currentFrameResource_ = nullptr;
};