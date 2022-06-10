
FORCEINLINE Quaternion::Quaternion() {
	_vec = DX::XMQuaternionIdentity();
}

FORCEINLINE Quaternion::Quaternion(const Vector3 &axis, const Scalar &angle) {
	_vec = DX::XMQuaternionRotationAxis(axis, angle);
}

FORCEINLINE Quaternion::Quaternion(float pitch, float yaw, float roll) {
	_vec = DX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

FORCEINLINE Quaternion::Quaternion(const Matrix3 &matrix) {
	_vec = XMQuaternionRotationMatrix(matrix.operator DX::XMMATRIX());
}

FORCEINLINE Quaternion::Quaternion(DX::FXMVECTOR vec) {
	_vec = vec;
}

FORCEINLINE Quaternion::operator DX::XMVECTOR() const {
	return _vec;
}

FORCEINLINE Quaternion::operator Matrix3() const {
	return Matrix3(DX::XMMatrixRotationQuaternion(_vec));
}

FORCEINLINE Quaternion::operator Matrix4() const {
	return Matrix4(DX::XMMatrixRotationQuaternion(_vec));
}

FORCEINLINE Quaternion Quaternion::operator~() const {
	return Quaternion(DX::XMQuaternionConjugate(_vec));
}

FORCEINLINE Quaternion Quaternion::operator*(const Quaternion &rhs) const {
	return Quaternion(DX::XMQuaternionMultiply(rhs._vec, _vec));
}

FORCEINLINE Vector3 Quaternion::operator*(const Vector3 &rhs) const {
	return Vector3(DX::XMVector3Rotate(rhs._vec, _vec));
}

FORCEINLINE Quaternion &Quaternion::operator=(const Quaternion &rhs) {
	_vec = rhs._vec;
	return *this;
}

FORCEINLINE Quaternion &Quaternion::operator*=(const Quaternion &rhs) {
	*this = *this * rhs;
	return *this;
}

FORCEINLINE Quaternion normalize(const Quaternion &q) {
	return Quaternion(DX::XMQuaternionNormalize(q._vec));
}

FORCEINLINE Quaternion inverse(const Quaternion &q) {
	return Quaternion(DX::XMQuaternionInverse(q._vec));
}

FORCEINLINE Quaternion slerp(const Quaternion &lhs, const Quaternion &rhs, float t) {
	return normalize(Quaternion(DX::XMQuaternionSlerp(lhs._vec, rhs._vec, t)));
}

FORCEINLINE Quaternion lerp(const Quaternion &lhs, const Quaternion &rhs, float t) {
	return normalize(Quaternion(DX::XMVectorLerp(lhs._vec, rhs._vec, t)));
}

FORCEINLINE Quaternion conjugate(const Quaternion &q) {
	return Quaternion(DX::XMQuaternionConjugate(q._vec));
}

FORCEINLINE std::ostream & operator<<(std::ostream &os, const Quaternion &q) {
	os << '(' << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ')';
	return os;
}