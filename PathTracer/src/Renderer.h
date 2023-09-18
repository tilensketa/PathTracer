#pragma once

#include "Image.h"
#include "Scene.h"
#include "Ray.h"
#include "Camera.h"

class Renderer {
public:
	struct Settings {
		bool Accumulate = true;
		bool ShowEnvironment = true;
	};
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);

	void Render(const Scene& scene, const Camera& camera, Image& image, Image& accumulationImage);

	const Image GetData() const { return *m_Image; }

	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }

private:
	struct HitPayload {
		float HitDistance;
		glm::vec3 WorldNormal;
		glm::vec3 WorldPosition;
		uint32_t ModelIndex;
		uint32_t MeshIndex;
		uint32_t TriangleIndex;
	};

	glm::vec3 PerPixel(uint32_t i);
	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, uint32_t modelIndex, uint32_t meshIndex, uint32_t triangleIndex);
	HitPayload Miss(const Ray& ray);

	glm::vec3 MapRayToHDRI(glm::vec3 rayDirection, const Texture& hdriImage);

private:
	Settings m_Settings;

	Image* m_Image = nullptr;
	Image* m_AccumulationImage = nullptr;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t m_FrameIndex = 1;

	uint32_t m_Width = 1000;
	uint32_t m_Height = 600;

	std::vector<uint32_t> m_ImageHorizontalIter;
	std::vector<uint32_t> m_ImageVerticalIter;

	bool m_First = true;
};