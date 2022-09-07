#pragma once
#include "RenderGraph/Pass/RenderQueuePass.h"
#include "RenderGraph/Technique/TechniqueType.hpp"

class ShadowApp;

namespace ShadowRgph {

constexpr rgph::TechniqueType kOpaque{ 1 };
constexpr rgph::TechniqueType kShadow{ 2 };

inline std::string ClearRTAndDSPass			= "ClearRTAndDS";
inline std::string ClearCSMShadowMapPass	= "ClearCSMShadowMapPass";
inline std::string ShadowPass				= "ShadowPass";
inline std::string OpaquePass				= "OpaquePass";
inline std::string SkyBoxPass				= "SkyBoxPass";
inline std::string PresentPass				= "PresentPass";

}


struct ShadowPass : rgph::RenderQueuePass {
	explicit ShadowPass(const std::string &passName);
};

struct OpaquePass : rgph::RenderQueuePass {
	OpaquePass(const std::string &passName);
	rgph::PassResourcePtr<dx12lib::ITextureResource2DArray> pShadowMapArray;
};

std::shared_ptr<rgph::RenderGraph> createShadowRenderGraph(ShadowApp *pApp, dx12lib::DirectContextProxy pDirectCtx);