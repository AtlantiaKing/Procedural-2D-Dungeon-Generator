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

void Dungeon::SetRoomBounds(int minSize, int maxSize)
{
	m_Generator.SetRoomSizeBounds(minSize, maxSize);
}

void Dungeon::SetGenerationState(bool isSlowlyGenerating)
{
	m_Generator.SetGenerationState(isSlowlyGenerating);
}

void Dungeon::Update()
{
	m_Generator.Update(m_Rooms);
}

void Dungeon::Draw() const
{
	for (const DungeonRoom& room : m_Rooms)
	{
		room.Draw();
	}
}
