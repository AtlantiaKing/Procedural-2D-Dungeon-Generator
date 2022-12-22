//---------------------------
// Includes
//---------------------------
#include "Dungeon.h"
#include "DungeonGenerator.h"

//---------------------------
// Constructor & Destructor
//---------------------------
Dungeon::Dungeon()
{
	DungeonGenerator::GenerateDungeon(m_Rooms);
}

//---------------------------
// Member functions
//---------------------------
void Dungeon::Draw() const
{
	for (const DungeonRoom& room : m_Rooms)
	{
		room.Draw();
	}
}
