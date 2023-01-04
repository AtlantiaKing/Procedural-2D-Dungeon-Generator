#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "DataTypes.h"

//-----------------------------------------------------------------
// Camera Class
//-----------------------------------------------------------------
class Camera
{
public:
	static Camera* GetSingleton();

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void SetCenter(const Vector2& center);
	void AddZoom(float value);
	void Move(int x, int y);

	Vector2 ScalePoint(const Vector2& point) const;
	int ScaleSize(int size) const;
private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	static Camera* m_pCamera;

	float m_Zoom{ 1 };
	Vector2 m_Center{};
	Vector2 m_Position{};
};

#define CAMERA (Camera::GetSingleton())