#pragma once
#include "D3D/Shader/ShaderCommon.h"
#include "RenderGraph/Material/Material.h"
#include "D3D/Model/MeshModel/MeshModel.h"


class ShadowApp;

namespace rgph {
	class SubPass;
}

struct CbObject {
	d3d::MaterialData gMaterialData = d3d::MaterialData::defaultMaterialData;
	Math::float4x4	  gMatTexCoord = Math::float4x4::identity();
};

class ShadowMaterial : public rgph::Material {
public:
	explicit ShadowMaterial(dx12lib::IDirectContext &directCtx, std::shared_ptr<dx12lib::Texture> pDiffuseTex);
	FRConstantBufferPtr<CbObject> _pCbObject;
public:
	static void init(ShadowApp *pApp);
	static d3d::MeshModel::MaterialCreator getShadowMaterialCreator(dx12lib::DirectContextProxy pDirectCtx);
	static inline std::shared_ptr<dx12lib::GraphicsPSO> pOpaquePso;
	static inline std::shared_ptr<dx12lib::GraphicsPSO> pShadowPso;
	static inline std::shared_ptr<rgph::SubPass> pOpaqueSubPass;
	static inline std::shared_ptr<rgph::SubPass> pShadowSubPass;
	static void opaqueSubPassOnBindCallback(const rgph::SubPass *pSubPass, dx12lib::IGraphicsContext &graphicsCtx);
};
