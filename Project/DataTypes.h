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
	Vector2 operator*(int multiplier) const
	{
		return{ x * multiplier, y * multiplier };
	}
	Vector2 operator/(int divisor) const
	{
		return{ x / divisor, y / divisor };
	}
	bool operator==(const Vector2& other) const
	{
		return x == other.x && y == other.y;
	}

	int DistanceSqr(const Vector2& other) const
	{
		return (other.x - x) * (other.x - x) + (other.y - y) * (other.y - y);
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

struct Edge
{
	Vector2 p0;
	Vector2 p1;

	bool operator==(const Edge& other) const
	{
		return (p0 == other.p0 || p0 == other.p1) && (p1 == other.p0 || p1 == other.p1);
	}
	bool operator<(const Edge& other) const
	{
		return p0.DistanceSqr(p1) < other.p0.DistanceSqr(other.p1);
	}
};

enum class TreeConnectionState
{
	Not_Connected,
	Connected,
	Loop
};

struct Tree
{
	std::vector<Edge> edges{};

	TreeConnectionState IsConnected(const Edge& edge) const
	{
		// Which points are connected
		bool connectedToP0{};
		bool connectedToP1{};

		// For each edge
		for (const Edge& edgeInTree : edges)
		{
			// Check if a point of the checking edge is on an edge of the tree
			bool edgeConnectedToP0{ edgeInTree.p0 == edge.p0 || edgeInTree.p1 == edge.p0 };
			bool edgeConnectedToP1{ edgeInTree.p0 == edge.p1 || edgeInTree.p1 == edge.p1 };

			if (edgeConnectedToP0) connectedToP0 = true;
			if (edgeConnectedToP1) connectedToP1 = true;
		}

		if (connectedToP0 && connectedToP1)
		{
			// If the edge is connected with both points, return that this edge would make the tree loop
			return TreeConnectionState::Loop;
		}
		else if (connectedToP0 || connectedToP1)
		{
			// If the edge is connected with only one point, return that this edge is connected to the tree
			return TreeConnectionState::Connected;
		}

		// Else, the edge is not connected to this tree
		return TreeConnectionState::Not_Connected;
	}

	void Merge(const Tree& other)
	{
		edges.reserve(edges.size() + other.edges.size());
		edges.insert(edges.end(), other.edges.begin(), other.edges.end());
	}
};