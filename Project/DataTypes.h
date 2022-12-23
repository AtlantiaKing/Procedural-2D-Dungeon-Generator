#pragma once
#include "GameEngine.h"
#include "GameDefines.h"

struct Vector2
{
	int x{};
	int y{};

	Vector2 operator+(const Vector2& other) const
	{ 
		return Vector2{ x + other.x, y + other.y }; 
	}
	Vector2 operator-(const Vector2& other) const
	{
		return Vector2{ x - other.x, y - other.y }; 
	}
};

struct Color
{
	int r{};
	int g{};
	int b{};

	COLORREF GetColor() const { return RGB(r, g, b); };
};