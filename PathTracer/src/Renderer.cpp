#include "Renderer.h"
#include "Mesh.h"

#include "Utils.h"

#include <algorithm>
#include <execution>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/compatibility.hpp>

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

	glm::vec3 incomingLight(0.0f);
	glm::vec3 rayColor(1.0f);

	uint32_t bounces = 10;
	for (uint32_t k = 0; k < bounces; k++)
	{
		HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f) {
			const Texture hdriImage = m_ActiveScene->EnvironmentImages[m_ActiveScene->SelectedEnvironment];
			glm::vec3 environmentColor = MapRayToHDRI(ray.Direction, hdriImage);
			rayColor *= environmentColor * m_ActiveScene->EnvironmetStrength;
			return rayColor;
		}
		const Model& model = m_ActiveScene->Models[payload.ModelIndex];
		const Mesh& mesh = model.GetMeshes()[payload.MeshIndex];
		const Material& material = mesh.GetMaterial();

		glm::vec2 barycentericCoordinates = CalculateBarycentricCoordinates(payload);

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		glm::vec3 diffuseDir = glm::normalize(payload.WorldNormal + Random::InUnitSphere());
		glm::vec3 specularDir = glm::reflect(ray.Direction, payload.WorldNormal);

		// Diffuse
		glm::vec3 diffuseColor;
		if (material.DiffuseTextureIndex >= 0) {
			Texture diffuseTexture = material.Textures[material.DiffuseTextureIndex];
			diffuseColor = diffuseTexture.SampleTexture(barycentericCoordinates);
		}
		else {
			diffuseColor = material.DiffuseColor;
		}

		// Specular
		float isSpecularBounce;
		glm::vec3 specularColor;
		if (material.SpecularTextureIndex >= 0) {
			Texture specularTexture = material.Textures[material.SpecularTextureIndex];
			isSpecularBounce = specularTexture.SampleTexture(barycentericCoordinates).r;
			specularColor = specularTexture.SampleTexture(barycentericCoordinates);
		}
		else {
			isSpecularBounce = material.Specular >= Random::Float(0.0f, 1.0f);
			specularColor = material.SpecularColor;
		}

		// Roughness
		float roughness;
		if (material.ShininessTextureIndex >= 0) {
			Texture shininessTexture = material.Textures[material.ShininessTextureIndex];
			roughness = shininessTexture.SampleTexture(barycentericCoordinates).r;
		}
		else {
			roughness = material.Roughness;
		}

		ray.Direction = glm::lerp(specularDir, diffuseDir, roughness);
		incomingLight += material.GetEmission() * rayColor;
		rayColor *= glm::lerp(diffuseColor, specularColor, isSpecularBounce);
	}
	return incomingLight;
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {
	int closestModelIndex = -1;
	int closestMeshIndex = -1;
	int triangleIndex = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (uint32_t i = 0; i < m_ActiveScene->Models.size(); i++)
	{
		const Model& model = m_ActiveScene->Models[i];
		for (uint32_t j = 0; j < model.GetMeshes().size(); j++)
		{
			const Mesh& mesh = model.GetMeshes()[j];
			float tNear, tFar;
			if (!RayIntersectsAABB(ray, mesh.GetAABB(), tNear, tFar)) {
				continue;
			}
			for (uint32_t k = 0; k < mesh.GetTriangles().size(); k++)
			{
				const Triangle& triangle = mesh.GetTriangles()[k];
				float t;
				if (RayIntersectsTriangle(ray, triangle, t)) {
					if (t < hitDistance) {
						hitDistance = t;
						closestModelIndex = i;
						closestMeshIndex = j;
						triangleIndex = k;
					}
				}
			}
		}
	}

	/*for (uint32_t i = 0; i < m_ActiveScene->Meshes.size(); i++)
	{
		const Mesh& mesh = m_ActiveScene->Meshes[i];
		float tNear, tFar;
		if (!RayIntersectsAABB(ray, mesh.GetAABB(), tNear, tFar)) {
			continue;
		}
		for (uint32_t j = 0; j < mesh.GetTriangles().size(); j++)
		{
			const Triangle& triangle = mesh.GetTriangles()[j];
			float t;
			if (RayIntersectsTriangle(ray, triangle, t)) {
				if (t < hitDistance) {
					hitDistance = t;
					triangleIndex = j;
					closestMeshIndex = i;
				}
			}
		}
	}*/

	if (closestMeshIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestModelIndex, closestMeshIndex, triangleIndex);
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

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, uint32_t modelIndex, uint32_t meshIndex, uint32_t triangleIndex) {
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ModelIndex = modelIndex;
	payload.MeshIndex = meshIndex;
	payload.TriangleIndex = triangleIndex;

	const Model& model = m_ActiveScene->Models[modelIndex];
	const Mesh& mesh = model.GetMeshes()[meshIndex];
	payload.WorldPosition = ray.Direction * hitDistance + ray.Origin;
	payload.WorldNormal = mesh.GetTriangles()[triangleIndex].A.Normal;

	return payload;
}
Renderer::HitPayload Renderer::Miss(const Ray& ray) {
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}

glm::vec2 Renderer::CalculateBarycentricCoordinates(const HitPayload& payload) {

	const Model& model = m_ActiveScene->Models[payload.ModelIndex];
	const Mesh& mesh = model.GetMeshes()[payload.MeshIndex];
	const Triangle& triangle = mesh.GetTriangles()[payload.TriangleIndex];

	// Calculate the barycentric coordinates
   	glm::vec3 a = triangle.B.Position - triangle.C.Position;
	glm::vec3 b = triangle.A.Position - triangle.C.Position;
	glm::vec3 p = payload.WorldPosition - triangle.C.Position;
	float A = glm::length(glm::cross(a, b) / 2.0f) / 2.0f;
	float A1 = glm::length(glm::cross(b, p) / 2.0f) / 2.0f;
	float A2 = glm::length(glm::cross(a, p) / 2.0f) / 2.0f;
	float u = A1 / A;
	float v = A2 / A;
	float w = 1.0f - u - v;
	glm::vec2 interpolatedTextureCoord = v * triangle.A.TexCoord + u * triangle.B.TexCoord + w * triangle.C.TexCoord;
	return interpolatedTextureCoord;
}

glm::vec3 Renderer::MapRayToHDRI(glm::vec3 ray_direction, const Texture& hdriImage) {
	// Convert ray direction to spherical coordinates
	float theta = std::acos(ray_direction.y);  // Zenith angle
	float phi = std::atan2(ray_direction.x, ray_direction.z) + glm::radians(m_ActiveScene->EnvironmentRotation);  // Azimuth angle

	return hdriImage.SampleSphericalTexture(phi, theta);
}