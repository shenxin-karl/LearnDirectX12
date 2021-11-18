#include <iostream>
#include "VectorHelper.h"

void float2Test() {
	using namespace vec;
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
	using namespace vec;
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
}

void float4Test() {
	using namespace vec;
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