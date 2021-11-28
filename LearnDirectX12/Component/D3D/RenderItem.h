#pragma once
#include <Windows.h>
#include "Math/MathHelper.h"
#include "Math/MatrixHelper.h"
#include "d3dulti.h"

namespace d3dUlti {

using namespace matrix;
constexpr int kNumFrameResources = 3;

struct RenderItem {
	RenderItem() = default;
	float4x4 world = MathHelper::identity4x4();
	int numFramesDirty = kNumFrameResources;
	UINT objCBIndex_ = -1;
	MeshGeometry *geometry_ = nullptr;
	D3D_PRIMITIVE_TOPOLOGY primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawInstance param
	UINT indexCount_ = 0;
	UINT startIndexLocation_ = 0;
	int BaseVertexLocation_ = 0;
};

}

