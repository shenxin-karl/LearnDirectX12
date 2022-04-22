#include "MathHelper.h"
#include "MathStd.hpp"

namespace Math {

DX::XMFLOAT4X4 MathHelper::identity4x4() {
	static float4x4 identity = float4x4(Matrix4::identity());
	return identity;
}

DX::XMFLOAT4X3 MathHelper::identity4x3() {
	static float4x3 identity = float4x3(Matrix4::identity());
	return identity;
}

DX::XMFLOAT3X4 MathHelper::identity3x4() {
	struct ObjectStore {
		DX::XMFLOAT3X4 object;
	public:
		ObjectStore() {
			DX::XMMATRIX matrix = DX::XMMatrixIdentity();
			DX::XMStoreFloat3x4(&object, matrix);
		}
		const auto &get() const {
			return object;
		}
	};
	static ObjectStore object;
	return object.get();
}

DX::XMFLOAT3X3 MathHelper::identity3x3() {
	static float3x3 identity = float3x3(Matrix3::identity());
	return identity;
}

}
