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
	Vector2& operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	Vector2& operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	Vector2 operator*(int multiplier)
	{
		return{ x * multiplier, y * multiplier };
	}
	Vector2 operator/(int divisor)
	{
		return{ x / divisor, y / divisor };
	}

	int DistanceSqr(const Vector2& other)
	{
		return (other.x - x) * (other.x - x) + (other.y - y) * (other.y) - y;
	}

	float ToDirection()
	{
		float length{ sqrtf(static_cast<float>(x * x + y * y)) };
		float xRanged{ static_cast<float>(x) / length };
		float yRanged{ static_cast<float>(y) / length };

		x = static_cast<int>(abs(xRanged) > 0.5f);
		if (xRanged < 0)
		{
			x *= -1;
		}
		y = static_cast<int>(abs(yRanged) > 0.5f);
		if (yRanged < 0)
		{
			y *= -1;
		}

		return length;
	}
};

struct Color
{
	int r{};
	int g{};
	int b{};

	COLORREF GetColor() const { return RGB(r, g, b); };
};

struct Triangle
{
	int first{}, second{}, third{};
};