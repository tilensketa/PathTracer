#include "Renderer.h"
#include "Mesh.h"

#include "Utils.h"

#include <algorithm>
#include <execution>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/color_space.hpp>

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
			if (m_Settings.ShowEnvironment) {
				const Texture hdriImage = m_ActiveScene->EnvironmentImages[m_ActiveScene->SelectedEnvironment];
				glm::vec3 environmentColor = MapRayToHDRI(ray.Direction, hdriImage);
				rayColor *= environmentColor * m_ActiveScene->EnvironmetStrength;
				return rayColor;
			}
			break;
		}
		const Model& model = m_ActiveScene->Models[payload.ModelIndex];
		const Mesh& mesh = model.GetMeshes()[payload.MeshIndex];
		const Triangle& triangle = mesh.GetTriangles()[payload.TriangleIndex];
		const Material& material = mesh.GetMaterial();

		glm::vec2 interpolatedTextureCoordinates = triangle.CalculateTextureCoordinates(payload.WorldPosition);

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		glm::vec3 diffuseDir = glm::normalize(payload.WorldNormal + Random::InUnitSphere());
		glm::vec3 specularDir = glm::reflect(ray.Direction, payload.WorldNormal);

		// Diffuse
		glm::vec3 diffuseColor;
		if (material.DiffuseTextureIndex >= 0) {
			Texture diffuseTexture = material.Textures[material.DiffuseTextureIndex];
			diffuseColor = diffuseTexture.SampleTexture(interpolatedTextureCoordinates);
		}
		else {
			diffuseColor = material.DiffuseColor;
		}

		// Specular
		float isSpecularBounce;
		glm::vec3 specularColor;
		if (material.SpecularTextureIndex >= 0) {
			Texture specularTexture = material.Textures[material.SpecularTextureIndex];
			isSpecularBounce = specularTexture.SampleTexture(interpolatedTextureCoordinates).r;
			specularColor = specularTexture.SampleTexture(interpolatedTextureCoordinates);
		}
		else {
			isSpecularBounce = material.Specular >= Random::Float(0.0f, 1.0f);
			specularColor = material.SpecularColor;
		}

		// Roughness
		float roughness;
		if (material.ShininessTextureIndex >= 0) {
			Texture shininessTexture = material.Textures[material.ShininessTextureIndex];
			glm::vec3 rgb = shininessTexture.SampleTexture(interpolatedTextureCoordinates);
			glm::vec3 hsv = glm::hsvColor(rgb);
			roughness = hsv.b;
		}
		else {
			roughness = material.Roughness;
		}

		// Normal
		if (material.NormalTextureIndex >= 0) {
			Texture normalTexture = material.Textures[material.NormalTextureIndex];
			glm::vec3 rgb = normalTexture.SampleTexture(interpolatedTextureCoordinates);
			ray.Direction = rgb;
		}
		else {
			ray.Direction = glm::lerp(specularDir, diffuseDir, roughness);
		}

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
			if (!mesh.GetAABB().Intersects(ray.Origin, ray.Direction))
				continue;
			for (uint32_t k = 0; k < mesh.GetTriangles().size(); k++)
			{
				const Triangle& triangle = mesh.GetTriangles()[k];
				float t;
				if(triangle.Intersects(ray.Origin, ray.Direction, t)) {
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

	if (closestMeshIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestModelIndex, closestMeshIndex, triangleIndex);
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

glm::vec3 Renderer::MapRayToHDRI(glm::vec3 rayDirection, const Texture& hdriImage) {
	// Convert ray direction to spherical coordinates
	float theta = std::acos(rayDirection.y);  // Zenith angle
	float phi = std::atan2(rayDirection.x, rayDirection.z) + glm::radians(m_ActiveScene->EnvironmentRotation);  // Azimuth angle

	return hdriImage.SampleSphericalTexture(phi, theta);
}