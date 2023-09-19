#pragma once

#include "Triangle.h"
#include "AABB.h"
#include "Utils.h"

#include <glm/glm.hpp>

#include <vector>

struct BVHNode {
	AABB BoundingBox;
	BVHNode* Left = nullptr;
	BVHNode* Right = nullptr;
	std::vector<Triangle> Triangles;
	bool IsLeaf = false;

	bool Intersects(const glm::vec3& origin, const glm::vec3& direction) const {
		return BoundingBox.IntersectsWithRay(origin, direction);
	}
};

static BVHNode* CreateLeafNode(const std::vector<Triangle>& triangles) {
	BVHNode* node = new BVHNode;
	// Compute the bounding box that encloses all triangles in this node.
	float max = std::numeric_limits<float>::max();
	float min = std::numeric_limits<float>::min();
	glm::vec3 minVert = glm::vec3(max, max, max);
	glm::vec3 maxVert = glm::vec3(-max, -max, -max);
	for (uint32_t i = 0; i < triangles.size(); i++)
	{
		const Triangle& triangle = triangles[i];
		for (uint32_t j = 0; j < 3; j++)
		{
			Vertex vert;
			if (j == 0)
				vert = triangle.A;
			else if (j == 1)
				vert = triangle.B;
			else if (j == 2)
				vert = triangle.C;
			minVert.x = std::min(minVert.x, vert.Position.x);
			minVert.y = std::min(minVert.y, vert.Position.y);
			minVert.z = std::min(minVert.z, vert.Position.z);

			maxVert.x = std::max(maxVert.x, vert.Position.x);
			maxVert.y = std::max(maxVert.y, vert.Position.y);
			maxVert.z = std::max(maxVert.z, vert.Position.z);
		}
	}
	node->BoundingBox = AABB(maxVert, minVert);
	node->IsLeaf = true;
	node->Triangles = triangles;
	return node;
}

// Recursive function to build the BVH.
static BVHNode* BuildBVH(std::vector<Triangle>& triangles, uint32_t trianglesInLeaf) {
	if (triangles.size() <= trianglesInLeaf) {
		return CreateLeafNode(triangles);
	}

    BVHNode* node = new BVHNode;
	node->Triangles = triangles;

    // Compute the bounding box that encloses all triangles in this node.
	float max = std::numeric_limits<float>::max();
	float min = std::numeric_limits<float>::min();
	glm::vec3 minVert = glm::vec3(max, max, max);
	glm::vec3 maxVert = glm::vec3(-max, -max, -max);
	for (uint32_t i = 0; i < triangles.size(); i++)
	{
		const Triangle& triangle = triangles[i];
		for (uint32_t j = 0; j < 3; j++)
		{
			Vertex vert;
			if(j == 0)
				vert = triangle.A;
			else if (j == 1)
				vert = triangle.B;
			else if (j == 2)
				vert = triangle.C;
			minVert.x = std::min(minVert.x, vert.Position.x);
			minVert.y = std::min(minVert.y, vert.Position.y);
			minVert.z = std::min(minVert.z, vert.Position.z);

			maxVert.x = std::max(maxVert.x, vert.Position.x);
			maxVert.y = std::max(maxVert.y, vert.Position.y);
			maxVert.z = std::max(maxVert.z, vert.Position.z);
		}
	}
	node->BoundingBox = AABB(maxVert, minVert);

    // Split the triangles into left and right sets
	uint32_t splitPlane = Random::Int(0, 2);
	float midX = 0;
	float midY = 0;
	float midZ = 0;
	for (uint32_t i = 0; i < triangles.size(); i++)
	{
		const Triangle& triangle = triangles[i];
		midX += triangle.Center.x;
		midY += triangle.Center.y;
		midZ += triangle.Center.z;
	}
	midX /= triangles.size();
	midY /= triangles.size();
	midZ /= triangles.size();

    std::vector<Triangle> leftTriangles;
    std::vector<Triangle> rightTriangles;
	for (uint32_t i = 0; i < triangles.size(); i++)
	{
		const Triangle& triangle = triangles[i];
		if (splitPlane == 0) {
			if (triangle.Center.x >= midX) {
				rightTriangles.push_back(triangle);
			}
			else {
				leftTriangles.push_back(triangle);
			}
		}
		if (splitPlane == 1) {
			if (triangle.Center.y >= midY) {
				rightTriangles.push_back(triangle);
			}
			else {
				leftTriangles.push_back(triangle);
			}
		}
		if (splitPlane == 2) {
			if (triangle.Center.z >= midZ) {
				rightTriangles.push_back(triangle);
			}
			else {
				leftTriangles.push_back(triangle);
			}
		}
	}

    // Recursively build the left and right child nodes.
	node->Left = BuildBVH(leftTriangles, trianglesInLeaf);
	node->Right = BuildBVH(rightTriangles, trianglesInLeaf);

    return node;
}