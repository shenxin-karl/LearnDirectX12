#include "SurfaceNet.h"
#include <tuple>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace voxel {

// 立方体的 12 条边
constexpr std::size_t kCubeEdges[12][2] = {
	{ 0u, 1u },
	{ 1u, 2u },
	{ 2u, 3u },
	{ 3u, 0u },
	{ 4u, 5u },
	{ 5u, 6u },
	{ 6u, 7u },
	{ 7u, 4u },
	{ 0u, 4u },
	{ 1u, 5u },
	{ 2u, 6u },
	{ 3u, 7u }
};

// 周围的点的偏移
constexpr int3 kNeighborGridPosition[7] = {
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 },
	{ 0, 1, 1 },
	{ 0, 0, 1 },
	{ 1, 0, 1 },
};

// 每个四边形需要的体素点
constexpr std::size_t kQuadNeedVertexIndex[3][3] = {
	{ 1, 2, 3 },
	{ 3, 4, 5 },
	{ 5, 6, 1 },
};

// 正对坐标轴, 逆时针
constexpr std::size_t kNeighborQuadIndices[3][6] = {
	{ 0, 1, 3,   3, 1, 2 },
	{ 0, 3, 5,   5, 3, 4 },
	{ 0, 5, 6,   0, 6, 1 },
};

// 背对坐标轴, 顺时针
constexpr std::size_t kRevNeighborQuadIndices[3][6] = {
	{ 0, 3, 1,   3, 2, 1 },
	{ 0, 5, 3,   5, 4, 3 },
	{ 0, 6, 5,   0, 1, 6 },
};

enum EdgeScalarAxis : std::size_t {
	ORIGIN  = 0,
	X_AXIST = 1,
	Y_AXIST = 2,
	Z_AXIST = 3,
};

// 计算边的偏导数, 用来判断是顺时针还是逆时针
std::array<float, 4> calcEdgeScalar(int x, int y, int z, const std::function<float(int, int, int)> &implicitFunction) {
	return {
		implicitFunction(x+0, y+0, z+0),	// origin
		implicitFunction(x+1, y+0, z+0),	// x-axis
		implicitFunction(x+0, y+1, z+0),	// y-axis
		implicitFunction(x+0, y+0, z+1),	// z-axis
	};
}

// 获取四边形的索引
decltype(auto) getQuadIndices(std::size_t i, const std::array<float, 4> &edgeSdfs) {
	float origin = edgeSdfs[ORIGIN];
	switch (i) {
	case 0:
		return (edgeSdfs[Z_AXIST] > origin) ? kNeighborQuadIndices[i] : kRevNeighborQuadIndices[i];
	case 1:
		return (edgeSdfs[X_AXIST] > origin) ? kNeighborQuadIndices[i] : kRevNeighborQuadIndices[i];
	case 2:
		return (edgeSdfs[Y_AXIST] > origin) ? kNeighborQuadIndices[i] : kRevNeighborQuadIndices[i];
	default:
		assert(false);
		return kNeighborQuadIndices[i];
	}
}

bool isEdgeActivate(float sdf0, float sdf1, float isovalue) {
	return (sdf0 >= isovalue) != (sdf1 >= isovalue);
}

com::MeshData surfaceNet(const std::function<float(int, int, int)> &implicitFunction,
	const com::Box3D &box,
	float isovalue) 
{
	Vector3 boxMax = Vector3(box.max);
	Vector3 boxMin = Vector3(box.min);
	Vector3 size = boxMax - boxMin;
	int sx = static_cast<int>(std::ceil(size.x));
	int sy = static_cast<int>(std::ceil(size.y));
	int sz = static_cast<int>(std::ceil(size.z));

	const auto getVoxelIndex = [=](const int3 &p) {
		return (p.y * sx * sz) + (p.z * sx) + p.x;
	};

	const auto getXYZFromIdx = [=, szSize=(sx*sz)](int index) {
		int x = index % sx;
		int z = (index / sx) % sz;
		int y = index / szSize;
		return std::make_tuple(x, y, z);
	};


	/* 我们访问体素网格中的每个体素, 也就是每个立方体规则的三维网格, 并确定哪些是相交的的隐函数定义的曲面.
	 * 为此我们寻找双极边缘, 双极边缘是它们的边缘. 顶点(v1, v2) 有关联的标量值, f 是隐式函数
	 *
	 * ( f(v1) > isovalue ) != ( f(v2) > isovalue )
	 *
	 * 双极边缘的立方体, 被标记为活动立方体. 每个活动立方体必须生成一个顶点
	 */ 
	int3  voxelCorrnerPositionArray[8];			// 8 个体素点的位置
	float voxelCorrnerSdfArray[8];				// 8 个体素的的sdf
	bool  edgeActivateArray[12];				// 12条边是否被激活
	int3  offset;

	com::MeshData mesh;
	std::unordered_map<int, std::size_t> activateVoxelMap;

	for (int y = 0; y < sy; ++y) {
		for (int z = 0; z < sz; ++z) {
			for (int x = 0; x < sx; ++x) {
				std::size_t index = 0;

				int3 position = { x, y, z };
				for (offset.x = 0; offset.x < 2; ++offset.x) {
					for (offset.y = 0; offset.y < 2; ++offset.y) {
						for (offset.z = 0; offset.z < 2; ++offset.z) {
							auto cp = position + offset;
							voxelCorrnerPositionArray[index] = cp;
							voxelCorrnerSdfArray[index] = implicitFunction(cp.x, cp.y, cp.z);
							++index;
						}
					}
				}

				// 判断是否在双极边缘上
				bool isVoxelActivate = false;
				for (std::size_t edge = 0; edge < 12; ++edge) {
					edgeActivateArray[edge] = isEdgeActivate(
						voxelCorrnerSdfArray[kCubeEdges[edge][0]],
						voxelCorrnerSdfArray[kCubeEdges[edge][1]],
						isovalue
					);
					isVoxelActivate = isVoxelActivate || edgeActivateArray[edge];
				}

				if (!isVoxelActivate)
					continue;

				Vector3 sumIntersectionPoint = Vector3(0.f);
				std::size_t intersectionCount = 0;
				for (std::size_t edge = 0; edge < 12; ++edge) {
					if (!edgeActivateArray[edge])
						continue;

					const auto &p0 = Vector3(voxelCorrnerPositionArray[kCubeEdges[edge][0]]);
					const auto &p1 = Vector3(voxelCorrnerPositionArray[kCubeEdges[edge][1]]);
					const float s0 = voxelCorrnerSdfArray[kCubeEdges[edge][0]];
					const float s1 = voxelCorrnerSdfArray[kCubeEdges[edge][1]];
					float t = (isovalue - s0) / (s1 - s0);
					Vector3 point = lerp(p0, p1, t);
					sumIntersectionPoint += point;		// 将所有的点求和
					++intersectionCount;
				}

				// 这里的点是局部坐标, 相对与 (0,0,0) ~ (sx, sy, sz) 访问内的局部坐标
				sumIntersectionPoint /= static_cast<float>(intersectionCount);
				//sumIntersectionPoint = float3{
				//	std::floor(sumIntersectionPoint.x) + 0.5f,
				//	std::floor(sumIntersectionPoint.y) + 0.5f,
				//	std::floor(sumIntersectionPoint.z) + 0.5f,
				//};
				sumIntersectionPoint += boxMax;

				int voxelIndex = getVoxelIndex(int3(x, y, z));
				std::size_t vertCount = mesh.vertices.size();
				mesh.vertices.emplace_back(sumIntersectionPoint);
				activateVoxelMap[voxelIndex] = vertCount;
			}
		}
	}

	/*
	 * 三角测量
	 * 三角测量我们不需要遍历所有的体素, 只需要遍历活跃的体素.
	 * 访问每一个活跃的体素, 看看有哪些邻居, 进行可能的三角化
	 * 在表面网络算法中, 当四个活动立方体共享一个公共边时生成一个四边形
	 * 因此在每次迭代中, 我们将查看相邻的体素, 如果它们都是活动的, 
	 * 如果它们都是活跃的, 将生成 2 个三角形组成的四边形
	 */

	// 越界判断
	auto isBoundaryCube = [bx=sx-1, by=sy-1, bz=sz-1](int x, int y, int z) {
		return (x >= bx || y >= by || z >= bz);
	};
	
	std::size_t neigborVoxelVertexIndex[7];		// 体素生成的顶点索引
	bool isVoxelActivate[7];					// 体素是否被激活
	for (auto [cubeIndex, vertIndex] : activateVoxelMap) {
		auto [x, y, z] = getXYZFromIdx(cubeIndex);
		if (isBoundaryCube(x, y, z))
			continue;

		int3 currentPostion = int3(x, y, z);
		neigborVoxelVertexIndex[0] = vertIndex;
		isVoxelActivate[0] = true;
		for (std::size_t i = 1; i < 7; ++i) {
			auto corrnerPos = currentPostion + kNeighborGridPosition[i];
			auto cubeIndex = getVoxelIndex(corrnerPos);
			isVoxelActivate[i] = false;
			if (auto iter = activateVoxelMap.find(cubeIndex); iter != activateVoxelMap.end()) {
				isVoxelActivate[i] = true;
				neigborVoxelVertexIndex[i] = iter->second;
			}
		}

		const auto &edgeSdfs = calcEdgeScalar(x, y, z, implicitFunction);
		for (std::size_t i = 0; i < 3; ++i) {
			const auto &quadVertexList = kQuadNeedVertexIndex[i];
			// 周围体素未激活, 不能组成四边形
			if (!isVoxelActivate[quadVertexList[0]] ||
				!isVoxelActivate[quadVertexList[1]] ||
				!isVoxelActivate[quadVertexList[2]]) {
				continue;
			}

			const auto &quadIndices = getQuadIndices(i, edgeSdfs);
			mesh.indices.insert(mesh.indices.end(), {
				std::uint32_t(neigborVoxelVertexIndex[quadIndices[0]]),
				std::uint32_t(neigborVoxelVertexIndex[quadIndices[1]]),
				std::uint32_t(neigborVoxelVertexIndex[quadIndices[2]]),
			});
			mesh.indices.insert(mesh.indices.end(), {
				std::uint32_t(neigborVoxelVertexIndex[quadIndices[3]]),
				std::uint32_t(neigborVoxelVertexIndex[quadIndices[4]]),
				std::uint32_t(neigborVoxelVertexIndex[quadIndices[5]]),
			});
		}
	}
	return mesh;
}

}

