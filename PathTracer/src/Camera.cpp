#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

Camera::Camera(float FOV, uint32_t width, uint32_t height, float near, float far)
	: m_FOV(FOV), m_ViewportWidth(width), m_ViewportHeight(height), m_NearClip(near), m_FarClip(far) {
	m_ForwardDirection = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 3);
	RecalculateProjection();
	RecalculateView();
	RecalculateRayDirections();
}

void Camera::OnResize(uint32_t width, uint32_t height) {
	if (m_ViewportWidth == width && m_ViewportHeight == height)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;
	RecalculateProjection();
	RecalculateRayDirections();
}

bool Camera::OnUpdate(GLFWwindow* window, float ts) {
	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);
	glm::vec2 mousePos(x, y);
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		return false;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	bool moved = false;

	//constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, m_UpDirection);

	float speed = GetMoveSpeed();

	// Movement
	if (glfwGetKey(window, GLFW_KEY_W))
	{
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_S))
	{
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A))
	{
		m_Position -= rightDirection * speed * ts;
		moved = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_D))
	{
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE))
	{
		m_Position += m_UpDirection * speed * ts;
		moved = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
	{
		m_Position -= m_UpDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved)
	{
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}

void Camera::RecalculateProjection() {
	float aspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;
	m_Projection = glm::perspective(glm::radians(m_FOV), aspectRatio, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView() {
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void Camera::RecalculateRayDirections() {
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++)
	{
		for (uint32_t x = 0; x < m_ViewportWidth; x++)
		{
			glm::vec2 coord = glm::vec2((float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight);
			coord = coord * 2.0f - 1.0f;

			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}