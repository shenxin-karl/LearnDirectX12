#pragma once
#include <string>
#include <d3d12.h>

namespace d3d {

struct VertexDataSemantic {
	std::string alias;
	std::string name;
	size_t	    slot;
	DXGI_FORMAT format;
public:
	friend bool operator==(const VertexDataSemantic &lhs, const VertexDataSemantic &rhs) {
		return lhs.slot == rhs.slot && lhs.format == rhs.format &&
			lhs.name == rhs.name;
	}
	friend bool operator!=(const VertexDataSemantic &lhs, const VertexDataSemantic &rhs) {
		return !(lhs == rhs);
	}
};

static inline VertexDataSemantic PositionSemantic{ "POSITION", "POSITION0", 0, DXGI_FORMAT_R32G32B32_FLOAT };
static inline VertexDataSemantic NormalSemantic{ "NORMAL", "NORMAL0", 1, DXGI_FORMAT_R32G32B32_FLOAT };
static inline VertexDataSemantic TangentSemantic{ "TEXCOORD", "TEXCOORD0", 3, DXGI_FORMAT_R32G32B32_FLOAT };
static inline VertexDataSemantic Texcoord0Semantic{ "TEXCOORD", "TEXCOORD0", 4, DXGI_FORMAT_R32G32_FLOAT };
static inline VertexDataSemantic Texcoord1Semantic{ "TEXCOORD1", "TEXCOORD01", 5, DXGI_FORMAT_R32G32_FLOAT };

static inline VertexDataSemantic SemanticList[] = {
	PositionSemantic,
	NormalSemantic,
	TangentSemantic,
	Texcoord0Semantic,
	Texcoord1Semantic
};

}
