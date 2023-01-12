#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "DungeonSolver.h"
#include "DataTypes.h"

//-----------------------------------------------------
// SlowDungeonSolver Class									
//-----------------------------------------------------
class SlowDungeonSolver final : public DungeonSolver
{
public:
	SlowDungeonSolver(std::shared_ptr<Dungeon> dungeon);

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	virtual bool Solve(bool saveShortestRoute = false) override;

	void Update(float elapsedSec);
	void Disable();

	void Draw() const;

private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	float m_AccuSec{};
	const float m_TimePerSec{ 0.5f };
	bool m_IsActive{};

	const int m_Size{ 10 };
	const Color m_Color{ 0, 255, 0 };
};

