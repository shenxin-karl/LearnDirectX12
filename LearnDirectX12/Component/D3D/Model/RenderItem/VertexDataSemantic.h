#pragma once
#include <string>
#include <d3d12.h>

namespace d3d {

struct VertexDataSemantic {
	std::string name;
	size_t		index;
	size_t	    slot;
	DXGI_FORMAT format;
public:
	friend bool operator==(const VertexDataSemantic &lhs, const VertexDataSemantic &rhs) {
		return lhs.slot == rhs.slot && lhs.index == rhs.index && 
			   lhs.name == rhs.name && lhs.format == rhs.format;
	}
	friend bool operator!=(const VertexDataSemantic &lhs, const VertexDataSemantic &rhs) {
		return !(lhs == rhs);
	}
	operator D3D12_INPUT_ELEMENT_DESC() const {
		D3D12_INPUT_ELEMENT_DESC desc;
		desc.SemanticName = name.c_str();
		desc.SemanticIndex = static_cast<UINT>(index);
		desc.Format = format;
		desc.InputSlot = static_cast<UINT>(slot);
		desc.AlignedByteOffset = 0;
		desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;
		return desc;
	}
};

static inline VertexDataSemantic PositionSemantic { "POSITION", 0, 0, DXGI_FORMAT_R32G32B32_FLOAT };
static inline VertexDataSemantic NormalSemantic   { "NORMAL",   0, 1, DXGI_FORMAT_R32G32B32_FLOAT };
static inline VertexDataSemantic TangentSemantic  { "TANGENT",  0, 2, DXGI_FORMAT_R32G32B32_FLOAT };
static inline VertexDataSemantic Texcoord0Semantic{ "TEXCOORD", 0, 3, DXGI_FORMAT_R32G32_FLOAT };
static inline VertexDataSemantic Texcoord1Semantic{ "TEXCOORD", 1, 4, DXGI_FORMAT_R32G32_FLOAT };

static inline VertexDataSemantic SemanticList[] = {
	PositionSemantic,
	NormalSemantic,
	TangentSemantic,
	Texcoord0Semantic,
	Texcoord1Semantic
};

}
