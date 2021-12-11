#include "BaseApp/BaseApp.h"
#include "Math/MathHelper.h"
#include "D3D/d3dutil.h"
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
	BoxApp();
	BoxApp(const BoxApp &) = delete;
	BoxApp &operator=(const BoxApp &) = delete;

	virtual bool initialize() override;
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual ~BoxApp() = default;
private:
	virtual void onResize(int width, int height) override;
	void buildDescriptorHeaps();
	void buildConstantsBuffers();
	void buildRootSignature();
	void buildShaderAndInputLayout();
	void buildBoxGeometry();
	void buildPSO();
	void processEvent();
	void onMouseMove(POINT mousePosition);
	void onMouseRPress();
	void onMouseRRelease();
	void onMouseWheel(float offset);
	void updateConstantBuffer() const;
	static DX::XMVECTOR toVector3(const DX::XMFLOAT3 &float3) noexcept;
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
	float theta_ = 0;
	float phi_ = 0;
	float radius_ = 10.0f;
	bool isMouseLeftPressed_ = false;
	POINT lastMousePos_ = { 0, 0 };
};