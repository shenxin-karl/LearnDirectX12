#include "MathHelper.h"

DX::XMFLOAT4X4 MathHelper::identity4x4() {
	struct ObjectStore {
		DX::XMFLOAT4X4 object;
	public:
		ObjectStore() {
			DX::XMMATRIX matrix = DX::XMMatrixIdentity();
			DX::XMStoreFloat4x4(&object, matrix);
		}
		const auto &get() const {
			return object;
		}
	};
	static ObjectStore object;
	return object.get();
}

DX::XMFLOAT4X3 MathHelper::identity4x3() {
	struct ObjectStore {
		DX::XMFLOAT4X3 object;
	public:
		ObjectStore() {
			DX::XMMATRIX matrix = DX::XMMatrixIdentity();
			DX::XMStoreFloat4x3(&object, matrix);
		}
		const auto &get() const {
			return object;
		}
	};
	static ObjectStore object;
	return object.get();
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
	struct ObjectStore {
		DX::XMFLOAT3X3 object;
	public:
		ObjectStore() {
			DX::XMMATRIX matrix = DX::XMMatrixIdentity();
			DX::XMStoreFloat3x3(&object, matrix);
		}
		const auto &get() const {
			return object;
		}
	};
	static ObjectStore object;
	return object.get();
}
