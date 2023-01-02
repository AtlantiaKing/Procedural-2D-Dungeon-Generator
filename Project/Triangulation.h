#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <vector>
#include "DataTypes.h"
#include "DungeonRoom.h"

//-----------------------------------------------------
// Triangulation Class									
//-----------------------------------------------------
class Triangulation
{
public:
	virtual void Triangulate(int screenSize, std::vector<DungeonRoom>& rooms) = 0;
	void Draw() const;
protected:
	std::vector<Triangle> m_Triangles{};
	std::vector<Vector2> m_Vertices{};

	int AddVertex(const Vector2& vertex);
	void AddTriangle(int first, int second, int third);
	void RemoveTriangle(size_t index);
};

