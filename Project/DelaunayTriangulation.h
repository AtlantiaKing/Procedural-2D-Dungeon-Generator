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
	virtual void Triangulate(int screenSize, std::vector<DungeonRoom>& rooms) override;

	void StartTriangulation(int screenSize);
	void AddPoint(const Vector2& point);
	void FinishTriangulation();
	void Clear();
private:
	bool IsInsideCircumcircle(const Triangle& triangle, int indice);
};

