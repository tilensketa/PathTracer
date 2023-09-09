#pragma once

#include <glm/glm.hpp>

#include <vector>

class Camera {
public:
	Camera(float FOV, uint32_t width, uint32_t height, float near, float far);

	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

	const uint32_t& GetWidth() const { return m_ViewportWidth; }
	const uint32_t& GetHeight() const { return m_ViewportHeight; }

private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();

private:
	glm::vec3 m_Position = glm::vec3(0.0f);
	glm::vec3 m_ForwardDirection = glm::vec3(0.0f);
	glm::vec3 m_UpDirection = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 m_View = glm::mat4(1.0f);
	glm::mat4 m_Projection = glm::mat4(1.0f);
	glm::mat4 m_InverseView = glm::mat4(1.0f);
	glm::mat4 m_InverseProjection = glm::mat4(1.0f);

	float m_FOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;

	std::vector<glm::vec3> m_RayDirections;

	float m_MoveSpeed;
	float m_RotationSpeed;

	uint32_t m_ViewportWidth;
	uint32_t m_ViewportHeight;
};