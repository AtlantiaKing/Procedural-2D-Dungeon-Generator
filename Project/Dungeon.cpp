//---------------------------
// Includes
//---------------------------
#include "Dungeon.h"
#include "DungeonGenerator.h"

//---------------------------
// Member functions
//---------------------------
void Dungeon::GenerateDungeon()
{
	m_Generator.GenerateDungeon(m_Rooms);
}

void Dungeon::SetInitialRadius(float radius)
{
	m_Generator.SetInitRadius(radius);
}

void Dungeon::SetInitialRoomCount(int roomCount)
{
	m_Generator.SetInitRoomCount(roomCount);
}

void Dungeon::Draw() const
{
	for (const DungeonRoom& room : m_Rooms)
	{
		room.Draw();
	}
}
