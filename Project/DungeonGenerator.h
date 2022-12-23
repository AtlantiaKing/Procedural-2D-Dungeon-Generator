#pragma once
#include <vector>
#include "DungeonRoom.h"

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

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

	void SetInitRadius(float initRadius) { m_InitRadius = initRadius; };
	void SetInitRoomCount(int initRoomCount) { m_InitRoomCount = initRoomCount; };
	void SetRoomSizeBounds(int minSize, int maxSize);
	
private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void CreateRoomsInCircle(std::vector<DungeonRoom>& rooms);
	bool SeperateRooms(std::vector<DungeonRoom>& rooms);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	const Vector2 m_Center{ 300, 300 };
	float m_InitRadius{ 100.0f };
	int m_InitRoomCount{ 200 };
	Vector2 m_RoomSizeBounds{ 15, 45 };

};
