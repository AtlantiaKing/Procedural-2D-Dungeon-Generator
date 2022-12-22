#pragma once
#include "GameEngine.h"
#include "GameDefines.h"

struct Vector2
{
	int x{};
	int y{};
};

struct Color
{
	int r{};
	int g{};
	int b{};

	COLORREF GetColor() const { return RGB(r, g, b); };
};