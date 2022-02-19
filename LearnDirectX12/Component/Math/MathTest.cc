#include <iostream>
#include "VectorHelper.h"

void float2Test() {
	using namespace Math;
	using namespace std;
	float2 f1;
	float2 f2(1);
	float2 f3(2, 2);
	cout << "f1: " << f1 << endl;
	cout << "f2: " << f2 << endl;
	cout << "f3: " << f3 << endl;
	f1 = float2(0);
	cout << "dot(f2, f3): " << dot(f2, f3) << endl;
	cout << "lengthSqr(f2): " << lengthSqr(f2) << endl;
	auto f4 = normalize(f3);
	cout << "length(f4): " << length(f4) << endl;
	f4 += 1.f;
	1 + f4;
	f4 += f1;
	f4 + f1;

	f4 -= 1.f;
	1 - f4;
	f4 -= f1;
	f4 - f1;

	f4 *= 1.f;
	1 * f4;
	f4 *= f1;
	f4 * f1;

	f4 /= 1.f;
	1 / f4;
	f4 /= f1;
	f4 /f1;
}

void float3Test() {
	using namespace Math;
	using namespace std;
	float3 f1;
	float3 f2(1);
	float3 f3(2, 2, 2);
	cout << "f1: " << f1 << endl;
	cout << "f2: " << f2 << endl;
	cout << "f3: " << f3 << endl;
	f1 = float3(0);
	cout << "dot(f2, f3): " << dot(f2, f3) << endl;
	cout << "lengthSqr(f2): " << lengthSqr(f2) << endl;
	auto f4 = normalize(f3);
	cout << "length(f4): " << length(f4) << endl;
	f4 += 1.f;
	1 + f4;
	f4 += f1;
	f4 + f1;

	f4 -= 1.f;
	1 - f4;
	f4 -= f1;
	f4 - f1;

	f4 *= 1.f;
	1 * f4;
	f4 *= f1;
	f4 *f1;

	f4 /= 1.f;
	1 / f4;
	f4 /= f1;
	f4 / f1;


	float3 p1(-0.5, 0, 0);
	float3 p2(0, +0.5, 0);
	float3 p3(+0.5, 0, 0);
	float3 e1 = p2 - p1;
	float3 e2 = p3 - p1;
	float3 myCrossRes = cross(e1, e2);
	std::cout << "myCrossRes: "	<< myCrossRes << std::endl;
	float3 dxCrossRes = float3(DX::XMVector3Cross(e1.toVec(), e2.toVec()));
	std::cout << "dxCrossRes: " << dxCrossRes << std::endl;
	std::cout << "math base axis cross test" << std::endl;
	float3 x = { 1, 0, 0 };
	float3 y = { 0, 1, 0 };
	float3 z = { 0, 0, 1 };
	std::cout << "xCy" << cross(x, y) << std::endl;
	std::cout << "yCx" << cross(y, x) << std::endl;
	std::cout << "xCz" << cross(x, z) << std::endl;
	std::cout << "zCx" << cross(z, x) << std::endl;
	std::cout << "yCz" << cross(y, z) << std::endl;
	std::cout << "zCy" << cross(z, y) << std::endl;
	std::cout << "dx base axis cross test" << std::endl;
	std::cout << "xCy" << float3(DX::XMVector3Cross(x.toVec(), y.toVec())) << std::endl;
	std::cout << "yCx" << float3(DX::XMVector3Cross(y.toVec(), x.toVec())) << std::endl;
	std::cout << "xCz" << float3(DX::XMVector3Cross(x.toVec(), z.toVec())) << std::endl;
	std::cout << "zCx" << float3(DX::XMVector3Cross(z.toVec(), x.toVec())) << std::endl;
	std::cout << "yCz" << float3(DX::XMVector3Cross(y.toVec(), z.toVec())) << std::endl;
	std::cout << "zCy" << float3(DX::XMVector3Cross(z.toVec(), y.toVec())) << std::endl;
}

void float4Test() {
	using namespace Math;
	using namespace std;
	float4 f1;
	float4 f2(1);
	float4 f3(2, 2, 2, 2);
	cout << "f1: " << f1 << endl;
	cout << "f2: " << f2 << endl;
	cout << "f3: " << f3 << endl;
	f1 = float4(0);
	cout << "dot(f2, f3): " << dot(f2, f3) << endl;
	cout << "lengthSqr(f2): " << lengthSqr(f2) << endl;
	auto f4 = normalize(f3);
	cout << "length(f4): " << length(f4) << endl;
	f4 += 1.f;
	1 + f4;
	f4 += f1;
	f4 + f1;

	f4 -= 1.f;
	1 - f4;
	f4 -= f1;
	f4 - f1;

	f4 *= 1.f;
	1 * f4;
	f4 *= f1;
	f4 *f1;

	f4 /= 1.f;
	1 / f4;
	f4 /= f1;
	f4 / f1;
}

int main() {
	float2Test();
	float3Test();
	float4Test();
}