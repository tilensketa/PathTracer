#include "Renderer.h"

#include "Utils.h"

#include <algorithm>
#include <execution>

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

	if (m_FrameIndex == 1)
		m_AccumulationImage->Clear();

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

	int bounces = 100;
	for (uint32_t k = 0; k < bounces; k++)
	{
		HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f) {
			glm::vec3 skyColor = glm::vec3(0.0f, 0.3f, 1.0f);
			//light += skyColor * contribution;
			break;
		}

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		contribution *= material.Albedo;
		light += material.GetEmmision();

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		//ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Random::Vec3(-0.5f, 0.5f));
		ray.Direction = glm::normalize(payload.WorldNormal + Random::InUnitSphere());
	}
	return light;
}


Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {

	int closestSphereIndex = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (uint32_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float t0 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		//float t1 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		if (t0 > 0.0f && t0 < hitDistance) {
			hitDistance = t0;
			closestSphereIndex = i;
		}
	}

	if (closestSphereIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphereIndex);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, uint32_t objectIndex) {
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& sphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - sphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += sphere.Position;

	return payload;
}
Renderer::HitPayload Renderer::Miss(const Ray& ray) {
	HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}