#include "Renderer.h"
#include "Mesh.h"

#include "Utils.h"

#include <algorithm>
#include <execution>

#include <glm/gtx/component_wise.hpp>

void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (m_Width == width && m_Height == height && !m_First)
		return;

	m_Width = width;
	m_Height = height;

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
	m_First = false;
}

void Renderer::Render(const Scene& scene, const Camera& camera, Image& image, Image& accumulationImage) {

	m_Image = &image;
	m_AccumulationImage = &accumulationImage;
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	m_Width = image.GetWidth();
	m_Height = image.GetHeight();

	if (m_FrameIndex == 1) {
		m_AccumulationImage->Clear();
	}

#define MT 1 //Multithreading
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
			[this, y](uint32_t x) {
					uint32_t i = y * m_Width + x;
					glm::vec3 color = PerPixel(i);
					glm::vec3 prevAccumulatedColor = m_AccumulationImage->GetPixel(i);
					glm::vec3 newAccumulatedColor = prevAccumulatedColor + color;
					m_AccumulationImage->SetPixel(i, newAccumulatedColor);
					newAccumulatedColor /= (float)m_FrameIndex;

					m_Image->SetPixel(i, newAccumulatedColor);
			});
		});
#else
	for (uint32_t y = 0; y < m_Height; y++)
	{
		for (uint32_t x = 0; x < m_Width; x++)
		{
			uint32_t i = y * m_Width + x;
			glm::vec3 color = PerPixel(i);
			glm::vec3 prevAccumulatedColor = m_AccumulationImage->GetPixel(i);
			glm::vec3 newAccumulatedColor = prevAccumulatedColor + color;
			m_AccumulationImage->SetPixel(i, newAccumulatedColor);
			newAccumulatedColor /= (float)m_FrameIndex;

			m_Image->SetPixel(i, newAccumulatedColor);
		}
	}

#endif

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}


glm::vec3 Renderer::PerPixel(uint32_t i) {
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[i];

	glm::vec3 light(0.0f);
	glm::vec3 contribution(1.0f);

	int bounces = 20;
	for (uint32_t k = 0; k < bounces; k++)
	{
		HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f) {
			glm::vec3 skyColor = glm::vec3(0.0f, 0.3f, 1.0f);
			//light += skyColor * contribution;
			break;
		}

		const Mesh& mesh = m_ActiveScene->Meshes[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[mesh.GetMaterialIndex()];
		contribution *= material.Albedo;
		light += material.GetEmmision();
		//light += material.EmissionPower * material.Albedo;
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::normalize(payload.WorldNormal + Random::InUnitSphere());
	}
	return light;
}


Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {
	int closestMeshIndex = -1;
	int triangleIndex = -1;
	float hitDistance = std::numeric_limits<float>::max();

	std::vector<uint32_t> intersectedMeshIndexes = BoundingIntersection(ray);
	if (intersectedMeshIndexes.empty())
		return Miss(ray);
	for (const uint32_t index : intersectedMeshIndexes)
	{
		const Mesh& mesh = m_ActiveScene->Meshes[index];
		for (uint32_t i = 0; i < mesh.GetTriangles().size(); i++)
		{
			const Triangle& triangle = mesh.GetTriangles()[i];
			float t;
			if (RayIntersectsTriangle(ray, triangle, t)) {
				if (t < hitDistance) {
					hitDistance = t;
					triangleIndex = i;
					closestMeshIndex = index;
				}
			}
		}
	}

	if (closestMeshIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestMeshIndex, triangleIndex);
}

std::vector<uint32_t> Renderer::BoundingIntersection(const Ray& ray) {

	std::vector<uint32_t> result;
	for (uint32_t i = 0; i < m_ActiveScene->Meshes.size(); i++)
	{
		const Mesh& mesh = m_ActiveScene->Meshes[i];
		float tNear, tFar;
		if (RayIntersectsAABB(ray, mesh.GetAABB(), tNear, tFar)) {
			result.push_back(i);
		}
	}
	return result;
}

bool Renderer::RayIntersectsTriangle(const Ray& ray, const Triangle& triangle, float& outT) {
	const float EPSILON = 0.000001f;
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = triangle.B.Position - triangle.A.Position;
	edge2 = triangle.C.Position - triangle.A.Position;
	h = glm::cross(ray.Direction, edge2);
	a = glm::dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return false;

	f = 1.0f / a;
	s = ray.Origin - triangle.A.Position;
	u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f)
		return false;

	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.Direction, q);

	if (v < 0.0f || u + v > 1.0f)
		return false;

	outT = f * glm::dot(edge2, q);

	if (outT > EPSILON)
		return true;

	return false;
}

bool Renderer::RayIntersectsAABB(const Ray& ray, const AABB& box, float& outTNear, float& outTFar) {
	glm::vec3 invDir = 1.0f / ray.Direction;
	glm::vec3 t1 = (box.Min - ray.Origin) * invDir;
	glm::vec3 t2 = (box.Max - ray.Origin) * invDir;

	outTNear = glm::compMax(glm::min(t1, t2));
	outTFar = glm::compMin(glm::max(t1, t2));

	return outTNear <= outTFar && outTFar >= 0;
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, uint32_t objectIndex, uint32_t triangleIndex) {
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Mesh& mesh = m_ActiveScene->Meshes[objectIndex];
	payload.WorldPosition = ray.Direction * hitDistance + ray.Origin;
	payload.WorldNormal = mesh.GetTriangles()[triangleIndex].A.Normal;

	return payload;
}
Renderer::HitPayload Renderer::Miss(const Ray& ray) {
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}