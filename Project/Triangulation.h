#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <vector>
#include "DataTypes.h"
#include "DungeonRoom.h"
#include <set>

//-----------------------------------------------------
// Triangulation Class									
//-----------------------------------------------------
class Triangulation
{
public:
	virtual void Triangulate(int screenSize, std::vector<DungeonRoom>& rooms) = 0;
	void Draw() const;
	void CreateSetOfEdges(std::set<Edge>& edges) const;
	virtual size_t GetSize() const;
protected:
	std::vector<Triangle> m_Triangles{};
	std::vector<std::pair<Vector2, int>> m_Vertices{};

	int AddVertex(const Vector2& vertex, int dungeonRoomIdx);
	void AddTriangle(int first, int second, int third);
	void RemoveTriangle(size_t index);
};

