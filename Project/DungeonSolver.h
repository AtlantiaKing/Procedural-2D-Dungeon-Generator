#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <vector>
#include <stack>

//-----------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------
class Dungeon;

//-----------------------------------------------------
// DungeonSolver Class									
//-----------------------------------------------------
class DungeonSolver
{
public:
	DungeonSolver(Dungeon* dungeon);
	virtual ~DungeonSolver() = default;

	//---------------------------
	// Disabling copy/move constructors and assignment operators   
	//---------------------------
	DungeonSolver(const DungeonSolver& other) = delete;
	DungeonSolver(DungeonSolver&& other) noexcept = delete;
	DungeonSolver& operator=(const DungeonSolver& other) = delete;
	DungeonSolver& operator=(DungeonSolver&& other) noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	virtual bool Solve();

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	bool HasDiscovered(int roomIdx) const;
	bool SolveStep();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Dungeon* m_pDungeon{};

	std::stack<int> m_PreviousRooms{};
	std::vector<int> m_DiscoveredRooms{};
	int m_CurRoom{};
};

