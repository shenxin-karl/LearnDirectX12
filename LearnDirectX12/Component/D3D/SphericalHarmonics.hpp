#include "Math/MathHelper.h"
#include <cmath>

namespace d3d {

using namespace Math;

template<size_t L, size_t M>
struct SHBasisFunction;

#define DECLARE_SH_BASIS_FUNCTION(L, M, COEF, EXPR)				\
template<>														\
struct SHBasisFunction<L, M> {									\
	constexpr inline static float PI = 3.141592654f;			\
	static float eval(const float3 &v) noexcept {				\
		using std::sqrt;										\
		static const float coef = static_cast<float>(COEF);		\
		[[maybe_unused]] const float x = v.x;					\
		[[maybe_unused]] const float y = v.y;					\
		[[maybe_unused]] const float z = v.z;					\
		return coef * (EXPR);									\
	}															\
}

DECLARE_SH_BASIS_FUNCTION( (0), (+0), (0.5f * sqrt(1.f / PI)), (1.f) );

DECLARE_SH_BASIS_FUNCTION( (1), (-1), (sqrt(3.f / (4.f * PI))), (y) );
DECLARE_SH_BASIS_FUNCTION( (1), (+0), (sqrt(3.f / (4.f * PI))), (z) );
DECLARE_SH_BASIS_FUNCTION( (1), (+1), (sqrt(3.f / (4.f * PI))), (x) );

DECLARE_SH_BASIS_FUNCTION( (2), (-2), (0.5f * sqrt(15.f / PI)), (x * y) );
DECLARE_SH_BASIS_FUNCTION( (2), (-1), (0.5f * sqrt(15.f / PI)), (y * z) );
DECLARE_SH_BASIS_FUNCTION( (2), (+0), (0.25f * sqrt(5.f / PI)), (3.f * z * z - 1.f) );
DECLARE_SH_BASIS_FUNCTION( (2), (+1), (0.5f * sqrt(15.f / PI)), (x * z) );
DECLARE_SH_BASIS_FUNCTION( (2), (+2), (0.25f * sqrt(15.f / PI)), (x * x - y * y) );

DECLARE_SH_BASIS_FUNCTION( (3), (-3), (0.25f * sqrt(35.f / 2.f * PI)), (y * (3.f * x * x - y * y)) );
DECLARE_SH_BASIS_FUNCTION( (3), (-2), (0.5f * sqrt(105.f / PI)), (x * y * z) );
DECLARE_SH_BASIS_FUNCTION( (3), (-1), (0.25f * sqrt(21.f / 2.f * PI)), (y * (5.f * z * z - 1.f)) );
DECLARE_SH_BASIS_FUNCTION( (3), (+0), (0.25f * sqrt(7.f / PI)), (5.f * z * z * z - 3.f * z) );
DECLARE_SH_BASIS_FUNCTION( (3), (+1), (0.25f * sqrt(21.f / 2.f * PI)), (x * (5.f * z * z - 1.f)) );
DECLARE_SH_BASIS_FUNCTION( (3), (+2), (0.25f * sqrt(105.f / PI)), ((x * x - y * y) * z) );
DECLARE_SH_BASIS_FUNCTION( (3), (+3), (0.25f * sqrt(35.f / 2.f * PI)), (x * (x * x - 3.f * y * y)) );

DECLARE_SH_BASIS_FUNCTION( (4), (-4), (0.75f * sqrt(35.f / PI)), (x * y * (x * x - y * y)) );
DECLARE_SH_BASIS_FUNCTION( (4), (-3), (0.75f * sqrt(35.f / 2.f * PI)), (y * (3.f * x * x - y * y) * z) );
DECLARE_SH_BASIS_FUNCTION( (4), (-2), (0.75f * sqrt(5.f / PI)), (x * y * (7.f * z * z - 1.f)) );
DECLARE_SH_BASIS_FUNCTION( (4), (-1), (0.75f * sqrt(5.f / 2.f * PI)), (y * (7.f * z * z * z - 3.f * z)) );
DECLARE_SH_BASIS_FUNCTION( (4), (+0), (3.f / 16.f * sqrt(1.f / PI)), (35.f * z * z * z * z - 30.f * z * z + 3.f) );
DECLARE_SH_BASIS_FUNCTION( (4), (+1), (0.75f * sqrt(5.f / 2.f * PI)), (x * (7.f * z * z * z - 3.f * z)) );
DECLARE_SH_BASIS_FUNCTION( (4), (+2), (3.f / 8.f * sqrt(5.f / PI)), ((x * x - y * y) * (7.f * z * z - 1.f)) );
DECLARE_SH_BASIS_FUNCTION( (4), (+3), (0.75f * sqrt(35.f / 2.f * PI)), (x * (x * x - 3.f * y * y) * z) );
DECLARE_SH_BASIS_FUNCTION( (4), (+4), (3.f / 16.f * sqrt(35.f / PI)), (x * x * (x * x - 3.f * y * y) - y * y * (3.f * x * x - y * y)) );

union SH3 {
	struct {
		float4 y00;
		float4 y1_1; float4 y10; float4 y11;
		float4 y2_2; float4 y2_1; float4 y20; float4 y21; float4 y22;
	};
	float4 _m[9];
public:
	constexpr static auto getSHBasisFunc() noexcept {
		using BasisFuncType = decltype(&SHBasisFunction<0, 0>::eval);
		std::array<BasisFuncType, 9> funcList = {
			&SHBasisFunction<0, +0>::eval,
			&SHBasisFunction<1, -1>::eval,
			&SHBasisFunction<1, +0>::eval,
			&SHBasisFunction<1, +1>::eval,
			&SHBasisFunction<2, -2>::eval,
			&SHBasisFunction<2, -1>::eval,
			&SHBasisFunction<2, +0>::eval,
			&SHBasisFunction<2, +1>::eval,
			&SHBasisFunction<2, +2>::eval,
		};
		return funcList;
	}
};

}