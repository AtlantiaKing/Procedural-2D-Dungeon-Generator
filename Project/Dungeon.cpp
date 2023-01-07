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

	if (!m_Generator.IsDone())
	{
		m_Generator.RenderDebug();
	}
}

int Dungeon::GetInitialRoomCount() const
{
	return m_Generator.GetInitialRoomCount();
}

int Dungeon::GetInitialRadius() const
{
	return m_Generator.GetInitialRadius();
}
