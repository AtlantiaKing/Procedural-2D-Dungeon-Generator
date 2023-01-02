//---------------------------
// Includes
//---------------------------
#include "Triangulation.h"

//---------------------------
// Member functions
//---------------------------
void Triangulation::Draw() const
{
	GAME_ENGINE->SetColor(RGB(0, 255, 0));

	// For each triangle
	for (const Triangle& triangle : m_Triangles)
	{
		// Calculate each edge
		const Vector2& v0{ m_Vertices[triangle.first] };
		const Vector2& v1{ m_Vertices[triangle.second] };
		const Vector2& v2{ m_Vertices[triangle.third] };

		// Draw each edge
		GAME_ENGINE->DrawLine(v0.x, v0.y, v1.x, v1.y);
		GAME_ENGINE->DrawLine(v1.x, v1.y, v2.x, v2.y);
		GAME_ENGINE->DrawLine(v2.x, v2.y, v0.x, v0.y);
	}
}

int Triangulation::AddVertex(const Vector2& vertex)
{
	m_Vertices.push_back(vertex);
	return static_cast<int>(m_Vertices.size() - 1);
}

void Triangulation::AddTriangle(int first, int second, int third)
{
	m_Triangles.push_back({ first, second, third });
}

void Triangulation::RemoveTriangle(size_t index)
{
	m_Triangles[index] = m_Triangles[m_Triangles.size() - 1];
	m_Triangles.pop_back();
}
