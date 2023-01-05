#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <vector>
#include "DungeonRoom.h"
#include "DelaunayTriangulation.h"

//-----------------------------------------------------
// DungeonGenerator Class									
//-----------------------------------------------------
class DungeonGenerator final
{
public:
	DungeonGenerator();				// Constructor
	~DungeonGenerator() = default;	// Destructor

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void GenerateDungeon(std::vector<DungeonRoom>& rooms);
	void CreateCorridors(std::vector<DungeonRoom>& rooms);
	void Update(std::vector<DungeonRoom>& rooms);

	void SetSeed(int seed) { m_CurrentSeed = seed; };
	void SetInitRadius(int initRadius) { m_InitRadius = initRadius; };
	void SetInitRoomCount(int initRoomCount) { m_InitRoomCount = initRoomCount; };
	void SetRoomSizeBounds(int minSize, int maxSize);
	void SetGenerationState(bool isSlowlyGenerating) { m_IsSlowlyGenerating = isSlowlyGenerating; };
	void SetRoomSizeThreshold(int size) { m_RoomSizeThreshold = size; };

	void RenderDebug() const;
	bool IsDone() const;
	int GetInitialRoomCount() const;
	int GetInitialRadius() const;
	
private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void CreateRoomsInCircle(std::vector<DungeonRoom>& rooms);
	void CreateRoomInCircle(std::vector<DungeonRoom>& rooms);
	bool SeperateRooms(std::vector<DungeonRoom>& rooms);
	bool DiscardSmallRooms(std::vector<DungeonRoom>& rooms);
	bool DiscardBorderingRooms(std::vector<DungeonRoom>& rooms);
	void CreateMinimumSpanningTree();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	enum class GenerationCycleState
	{
		CIRCLE,
		SEPERATION,
		DISCARD_SMALL_ROOMS,
		DISCARD_BORDERING_ROOMS,
		TRIANGULATION,
		SPANNING_TREE_ALGORITHM,
		CORRIDORS,
		DONE
	};

	bool m_IsGenerating{};
	int m_CurrentSeed{ -1 };

	const Vector2 m_Center{ 300, 300 };
	int m_InitRadius{ 100 };
	int m_InitRoomCount{ 200 };
	Vector2 m_RoomSizeBounds{ 4, 40 };
	int m_RoomSizeThreshold{ 30 };
	int m_CurTriangulateRoom{};

	std::vector<DungeonRoom> m_DebugRooms{};
	std::vector<Edge> m_MinimumSpanningTree{};

	DelaunayTriangulation m_Triangulation{};
	GenerationCycleState m_CurrentGenerationState{};
	bool m_IsSlowlyGenerating{};

};
