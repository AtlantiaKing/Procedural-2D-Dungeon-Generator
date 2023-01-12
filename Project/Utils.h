#pragma once
#include <stdlib.h>

inline float RandomFloat(float min, float max)
{
	return (rand() % 1001 / 1000.0f) * (max - min) + min;
}

inline int RandomInt(int min, int max)
{
	return rand() % (max - min + 1) + min;
}