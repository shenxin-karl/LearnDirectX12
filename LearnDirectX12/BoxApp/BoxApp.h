#include "BaseApp/BaseApp.h"
#include "Math/MathHelper.h"
#include "D3D/d3dulti.h"
#include "D3D/UploadBuffer.h"

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

struct Vertex {
	DX::XMFLOAT3  position;
	DX::XMFLOAT4  color;
};

struct ObjectConstants {
	DX::XMFLOAT4X4 worldViewProj = MathHelper::identity4x4();
};

class BoxApp : public com::BaseApp {
public:
	BoxApp() = default;
	BoxApp(const BoxApp &) = delete;
	BoxApp &operator=(const BoxApp &) = delete;

	virtual bool initialize() override;
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer);
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer);
	virtual ~BoxApp() = default;
private:
	virtual void onResize(int width, int height) override;
	void buildDescriptorHeaps();
	void buildConstantsBuffers();
	void buildRootSignature();
	void buildShaderAndInputLayout();
	void buildBoxGeometry();
	void buildPSO();
	void onMouseMove();
	void onMouseRPress();
	void onMouseRRelease();
private:
	WRL::ComPtr<ID3D12RootSignature>				pRootSignature_;
	WRL::ComPtr<ID3D12DescriptorHeap>				pCbvHeap_;
	WRL::ComPtr<ID3D12PipelineState>				pPSO_;
	WRL::ComPtr<ID3DBlob>							pVsByteCode_;
	WRL::ComPtr<ID3DBlob>							pPsByteCode_;
	std::unique_ptr<UploadBuffer<ObjectConstants>>	pObjectCB_;
	std::unique_ptr<MeshGeometry>					pBoxGeo_;
	std::vector<D3D12_INPUT_ELEMENT_DESC>			inputLayout_;
	DX::XMFLOAT4X4 worldMat_ = MathHelper::identity4x4();
	DX::XMFLOAT4X4 viewMat_ = MathHelper::identity4x4();
	DX::XMFLOAT4X4 projMat_ = MathHelper::identity4x4();
	float theta_ = 1.5f * DX::XM_PI;
	float phi_ = DX::XM_PIDIV4;
	float radius_ = 5.0f;
	POINT lastMousePos_;
};