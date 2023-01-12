#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <memory>
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
	DungeonSolver(std::shared_ptr<Dungeon> dungeon);
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
	void SetNeedAllKeys(bool needAllKeys) { m_NeedAllKeys = needAllKeys; }

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	bool HasDiscovered(int roomIdx) const;
	void SaveShortestRoute() const;
	
protected:
	//-------------------------------------------------
	// Protected member functions								
	//-------------------------------------------------
	bool SolveStep();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	std::shared_ptr<Dungeon> m_pDungeon{};

	bool m_NeedAllKeys{};
	std::vector<int> m_ForcedPath{};
	static std::vector<int> m_ShortestPath;
	std::vector<int> m_TotalPath{};
	std::stack<int> m_PreviousRooms{};
	std::vector<int> m_DiscoveredRooms{};
	int m_CurRoom{};
	int m_NrKeys{};
};

