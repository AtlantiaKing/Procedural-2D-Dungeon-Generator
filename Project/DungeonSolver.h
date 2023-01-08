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
	virtual bool Solve(bool saveShortestRoute = false);

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	bool HasDiscovered(int roomIdx) const;
	void SaveShortestRoute();
	
protected:
	//-------------------------------------------------
	// Protected member functions								
	//-------------------------------------------------
	bool SolveStep();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Dungeon* m_pDungeon{};

	std::vector<int> m_ForcedPath{};
	static std::vector<int> m_ShortestPath;
	std::vector<int> m_TotalPath{};
	std::stack<int> m_PreviousRooms{};
	std::vector<int> m_DiscoveredRooms{};
	int m_CurRoom{};
	int m_NrKeys{};
};

