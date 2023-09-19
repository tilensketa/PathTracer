#pragma once

#include "Vertex.h"

struct Triangle {
	Vertex A;
	Vertex B;
	Vertex C;
	glm::vec3 Center;

	bool IntersectsWithRay(const glm::vec3& origin, const glm::vec3& direction, float& outT) const {
		const float EPSILON = 0.000001f;
		glm::vec3 edge1, edge2, h, s, q;
		float a, f, u, v;

		edge1 = B.Position - A.Position;
		edge2 = C.Position - A.Position;
		h = glm::cross(direction, edge2);
		a = glm::dot(edge1, h);

		if (a > -EPSILON && a < EPSILON)
			return false;

		f = 1.0f / a;
		s = origin - A.Position;
		u = f * glm::dot(s, h);

		if (u < 0.0f || u > 1.0f)
			return false;

		q = glm::cross(s, edge1);
		v = f * glm::dot(direction, q);

		if (v < 0.0f || u + v > 1.0f)
			return false;

		outT = f * glm::dot(edge2, q);

		if (outT > EPSILON)
			return true;

		return false;
	}

	glm::vec2 CalculateTextureCoordinates(const glm::vec3 position) const {
		// Calculate the barycentric coordinates
		glm::vec3 a = B.Position - C.Position;
		glm::vec3 b = A.Position - C.Position;
		glm::vec3 p = position - C.Position;
		float As = glm::length(glm::cross(a, b) / 2.0f) / 2.0f;
		float A1 = glm::length(glm::cross(b, p) / 2.0f) / 2.0f;
		float A2 = glm::length(glm::cross(a, p) / 2.0f) / 2.0f;
		float u = A1 / As;
		float v = A2 / As;
		float w = 1.0f - u - v;
		glm::vec2 interpolatedTextureCoord = v * A.TexCoord + u * B.TexCoord + w * C.TexCoord;
		return interpolatedTextureCoord;
	}
};