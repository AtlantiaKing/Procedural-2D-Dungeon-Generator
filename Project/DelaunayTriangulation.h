#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "Triangulation.h"

//-----------------------------------------------------
// DelaunayTriangulation Class									
//-----------------------------------------------------
class DelaunayTriangulation : public Triangulation
{
public:
	DelaunayTriangulation() = default;
	virtual ~DelaunayTriangulation() = default;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	virtual void Triangulate(std::vector<DungeonRoom>& rooms) override;

	void StartTriangulation();
	void AddPoint(const Vector2& point, int dungeonRoomIdx);
	void FinishTriangulation();
	void Clear();
	virtual size_t GetSize() const override;
private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	bool IsInsideCircumcircle(const Triangle& triangle, int indice);
};

