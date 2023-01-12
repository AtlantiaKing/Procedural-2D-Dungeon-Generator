//---------------------------
// Includes
//---------------------------
#include "Triangulation.h"
#include "Camera.h"

//---------------------------
// Member functions
//---------------------------
void Triangulation::Draw() const
{
	GAME_ENGINE->SetColor(RGB(0, 255, 0));

	// For each triangle
	for (const Triangle& triangle : m_Triangles)
	{
		// Calculate each vertex
		const Vector2& v0{ CAMERA->ScalePoint(m_Vertices[triangle.first].first) };
		const Vector2& v1{ CAMERA->ScalePoint(m_Vertices[triangle.second].first) };
		const Vector2& v2{ CAMERA->ScalePoint(m_Vertices[triangle.third].first) };

		// Draw each edge
		GAME_ENGINE->DrawLine(v0.x, v0.y, v1.x, v1.y);
		GAME_ENGINE->DrawLine(v1.x, v1.y, v2.x, v2.y);
		GAME_ENGINE->DrawLine(v2.x, v2.y, v0.x, v0.y);
	}
}

void Triangulation::CreateSetOfEdges(std::set<Edge>& edges) const
{
	// For each triangle
	for (const Triangle& triangle : m_Triangles)
	{
		// For each edge
		for (int i{}; i < 3; ++i)
		{
			// Create a new edge
			Edge newEdge{};

			// Set the correct vertices
			switch (i)
			{
			case 0:
			{
				newEdge.p0 = m_Vertices[triangle.first];
				newEdge.p1 = m_Vertices[triangle.second];
				break;
			}
			case 1:
			{
				newEdge.p0 = m_Vertices[triangle.second];
				newEdge.p1 = m_Vertices[triangle.third];
				break;
			}
			case 2:
			{
				newEdge.p0 = m_Vertices[triangle.third];
				newEdge.p1 = m_Vertices[triangle.first];
				break;
			}
			}

			// Add the new edge to the set
			edges.insert(newEdge);
		}
	}
}

size_t Triangulation::GetSize() const
{
	return m_Vertices.size();
}

int Triangulation::AddVertex(const Vector2& vertex, int dungeonRoomIdx)
{
	m_Vertices.push_back(std::make_pair(vertex, dungeonRoomIdx));

	// Return the index of the new vertex
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
