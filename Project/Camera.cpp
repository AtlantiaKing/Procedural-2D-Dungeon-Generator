//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Camera.h"

//-----------------------------------------------------------------
// Static Variable Initialization
//-----------------------------------------------------------------
Camera* Camera::m_pCamera{};

//-----------------------------------------------------------------
// Static Methods
//-----------------------------------------------------------------
Camera* Camera::GetSingleton()
{
	if (!m_pCamera) m_pCamera = new Camera();
	return m_pCamera;
}

//-----------------------------------------------------------------
// Member Methods
//-----------------------------------------------------------------
void Camera::SetCenter(const Vector2& center)
{
	m_Center = center;
}

void Camera::AddZoom(float value)
{
	m_Zoom = max(m_Zoom + value, 0.1f);
}

void Camera::Move(int x, int y)
{
	const float zoomMultiplier{ 1.0f / m_Zoom };
	m_Position.x += static_cast<int>(x * zoomMultiplier);
	m_Position.y += static_cast<int>(y * zoomMultiplier);
}

Vector2 Camera::ScalePoint(const Vector2& point) const
{
	return 
	{
		static_cast<int>((point.x - m_Center.x) * m_Zoom + m_Center.x + m_Position.x * m_Zoom),
		static_cast<int>((point.y - m_Center.y) * m_Zoom + m_Center.y + m_Position.y * m_Zoom)
	};
}

int Camera::ScaleSize(int size) const
{
	return static_cast<int>(size * m_Zoom);
}